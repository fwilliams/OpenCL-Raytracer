#include "opencl_ide_fix.h"

#define CONST_BRDF

// TODO: Move all of these somewhere better
#define NULL_TYPE_ID 0
#define SPHERE_TYPE_ID 1
#define TRIANGLE_TYPE_ID 2

#define MAX_REFLECTIONS 3

#define RAY_TRI_EPSILON 0.000001

struct Material {
	float3 reflectivity;
	float3 refraction;

#if defined CONST_BRDF				// All surfaces lambertian
	float3 color;
	
#elif defined BLINN_PHONG_BRDF			// Phong lighting (physically implausible)
	float3 kd;
	float3 ks;
	float exp;
	
#elif defined COOK_TORRANCE_BRDF	// Physically plausible lighting model (to be used with global illumination)
	float3 kd;
	float3 ks;
	float2 rougness;

#endif
};

struct Sphere {
	float radius;
	float3 position;
	uint materialId;
};

struct Triangle {
	float3 v1, v2, v3;
	uint materialId;
};

struct PointLight {
	float3 position;
	float3 power;
};

struct Ray {
	float3 origin;
	float3 direction;
};

bool rayTriangle(struct Ray* ray, global struct Triangle* tri, float* outT) {
	float3 e1, e2;
	float3 P, Q, T;
	float det, invDet, u, v;
	float t;

	//Find vectors for two edges sharing v1
	e1 = tri->v2 - tri->v1;
	e2 = tri->v3 - tri->v1;

	//Begin calculating determinant - also used to calculate u parameter
	P = cross(ray->direction, e2);
	det = dot(e1, P);

	//if determinant is near zero, ray lies in plane of triangle
	if(det > -RAY_TRI_EPSILON && det < RAY_TRI_EPSILON) {
		return false;
	}

	invDet = 1.0f / det;

	//calculate distance from V1 to ray origin
	T = ray->origin - tri->v1;

	//Calculate u parameter and test bound
	u = dot(T, P) * invDet;
	if(u < 0.0f || u > 1.0f) {
		//The intersection lies outside of the triangle
		return false;
	}

	//Prepare to test v parameter
	Q = cross(T, e1);

	//Calculate v parameter and test bound
	v = dot(ray->direction, Q) * invDet;
	if(v < 0.0f || u + v  > 1.0f) {
		//The intersection lies outside of the triangle
		return false;
	}

	t = dot(e2, Q) * invDet;

	if(t > RAY_TRI_EPSILON) {
		*outT = t;
		return true;
	}

	return false;
}

bool raySphere(struct Ray* r, global struct Sphere* s, float* t) {
	float3 rayToCenter = s->position - r->origin ;
	float dotProduct = dot(r->direction, rayToCenter);
	float d = dotProduct*dotProduct - dot(rayToCenter,rayToCenter)+s->radius*s->radius;
	float sqrtd = sqrt(d);

	if(d < 0) {
		return false;
	}

	*t = dotProduct - sqrtd;

	if(*t < 0) {
		*t = dotProduct + sqrtd;
		if(*t < 0) {
			return false;
		}
	}

	return true;
}

float intersect(
		struct Ray* ray,
		global struct Sphere* spheres,
		global struct Triangle* tris,
		int* index, uint* type) {
	float minT = MAX_RENDER_DISTANCE;

	for(int i = 0; i < NUM_SPHERES; i++) {
		float t;
		if(raySphere(ray, &spheres[i], &t)) {
			if(t < minT){
				minT = t;
				*type = SPHERE_TYPE_ID;
				*index = i;
			}
		}
	}

	for(int i = 0; i < NUM_TRIANGLES; i++) {
		float t;
		if(rayTriangle(ray, &tris[i], &t)) {
			if(t < minT){
				minT = t;
				*type = TRIANGLE_TYPE_ID;
				*index = i;
			}
		}
	}

	return minT;
}

float dt(float3 v1, float3 v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

inline float3 computeRadiance(
		float3* position, float3* normal, 
		global struct Material* material, 
		global struct PointLight* lights, 
		global struct Sphere* spheres, 
		global struct Triangle* tris) {
	
	float3 color = (float3){0.0, 0.0, 0.0};
	
	for(int j = 0; j < NUM_LIGHTS; j++) {
		float3 L = lights[j].position - *position;
		float distanceToLight = length(L);
		L = normalize(L);

		struct Ray shadowRay;
		shadowRay.origin = *position + L*0.001;
		shadowRay.direction = L;
		
		int index;
		uint type;
		float st = intersect(&shadowRay, spheres, tris, &index, &type);
		
		if(st > distanceToLight) {
			#if defined CONST_BRDF
			color += material->color*lights[j].power*max(0.0f, dot(*normal, L));
			
			#elif defined BLINN_PHONG_BRDF
			float3 H = normalize( L + normalize(-*position));
			color += lights[j].power * (material->kd*max(0.0f, dot(*normal, L)) + material->ks*pow(max(0.0f, dot(normal, H)), material->exp));
			#elif defined COOK_TORRANCE_BRDF

			#endif
		}
	}
	return color;
}

float3 reflect(float3 v, float3 n) {
	return v - 2.0f * dot(v, n) * n;
}

float3 doRaytrace(
		struct Ray* ray,
		global struct Sphere* spheres,
		global struct Triangle* tris,
		global struct PointLight* lights,
		global struct Material* materials, int traceDepth) {

	int intersectObjIndex = -1;
	uint intersectObjType = NULL_TYPE_ID;
	float t = intersect(ray, spheres, tris, &intersectObjIndex, &intersectObjType);

	float3 color = (float3){0.0f, 0.0f, 0.0f};

	if(t < MAX_RENDER_DISTANCE) {
		float3 intersectPos = ray->origin + ray->direction*t;
		float3 normal;

		global struct Material* m = &materials[0];

		if(intersectObjType == SPHERE_TYPE_ID) {
			normal = normalize(intersectPos - spheres[intersectObjIndex].position);
			m = &materials[spheres[intersectObjIndex].materialId];
		} else if(intersectObjType == TRIANGLE_TYPE_ID) {
			normal = normalize(
						cross(tris[intersectObjIndex].v2 - tris[intersectObjIndex].v1,
							  tris[intersectObjIndex].v2 - tris[intersectObjIndex].v3));
			m = &materials[tris[intersectObjIndex].materialId];
		}
		
		struct Ray reflectRay = *ray;
		global struct Material* reflectMat = m;
		float3 reflectNormal = normal;
		float3 reflectionFactor = 1.0f;
		
		for(int i = 0; i < MAX_REFLECTIONS; i++) {
			reflectionFactor *= reflectMat->reflectivity;
			
			reflectRay.direction = reflect(normalize(reflectRay.direction), reflectNormal);
			
			float rt = intersect(&reflectRay, spheres, tris, &intersectObjIndex, &intersectObjType);

			reflectRay.origin = reflectRay.origin + rt * reflectRay.direction;
			
			if(intersectObjType == SPHERE_TYPE_ID) {
				reflectNormal = normalize(reflectRay.origin - spheres[intersectObjIndex].position);
				reflectMat = &materials[spheres[intersectObjIndex].materialId];
			} else if(intersectObjType == TRIANGLE_TYPE_ID) {
				reflectNormal = normalize(
							cross(tris[intersectObjIndex].v2 - tris[intersectObjIndex].v1,
								  tris[intersectObjIndex].v2 - tris[intersectObjIndex].v3));
				reflectMat = &materials[tris[intersectObjIndex].materialId];
			} else {
				break;
			}
	
			color += reflectionFactor * computeRadiance(&reflectRay.origin, &reflectNormal, reflectMat, lights, spheres, tris);
		}
		
		color += computeRadiance(&intersectPos, &normal, m, lights, spheres, tris);
	}

	return clamp(color, 0.0, 1.0);
}

float3 matrixVectorMultiply(global float* matrix, float3* vector) { 
	float3 result;
	result.x = matrix[0]*((*vector).x)+matrix[4]*((*vector).y)+matrix[8]*((*vector).z)+matrix[12];
	result.y = matrix[1]*((*vector).x)+matrix[5]*((*vector).y)+matrix[9]*((*vector).z)+matrix[13];
	result.z = matrix[2]*((*vector).x)+matrix[6]*((*vector).y)+matrix[10]*((*vector).z)+matrix[14];
	return result;
}

kernel void raytrace(
		global const struct Triangle* triangles,
		global const struct Sphere* spheres,
		global const struct PointLight* lights,
		global const struct Material* materials,
		global float* viewMatrix,
		global write_only image2d_t res) {

	struct Ray ray;
	ray.origin = matrixVectorMultiply(viewMatrix, &((float3) {0.0f, 0.0f, 0.0f}));
	ray.direction = normalize((float3) {
		min(((float)get_global_id(0))/(float)get_global_size(0) - 0.5f, 1.0),
		min(-((float)get_global_id(1))/(float)get_global_size(1) + 0.5f, 1.0),
										-0.5});
	
	float3 color = doRaytrace(&ray, spheres, triangles, lights, materials, 0);
	
	write_imagef(res, (int2) {get_global_id(0), get_global_id(1)},
				 (float4){color.x, color.y, color.z, 1.0});
}