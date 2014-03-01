#include "opencl_ide_fix.h"

// TODO: Move all of these somewhere better
#define NULL_TYPE_ID 0
#define SPHERE_TYPE_ID 1
#define TRIANGLE_TYPE_ID 2

#define EPSILON 0.000001


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

struct Material {
	float3 reflection;
	float3 refraction;
	float3 diffuseColor;
};

struct Ray {
	float3 origin;
	float3 direction;
};

// TODO: #define these for increases performance
struct RenderParams {
	float maxRenderDist;
	uint numTris;
	uint numSpheres;
	uint numLights;
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
	if(det > -EPSILON && det < EPSILON) {
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

	if(t > EPSILON) {
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
		struct Ray* ray, global struct RenderParams* params,
		global struct Sphere* spheres,
		global struct Triangle* tris,
		int* index, uint* type) {
	float minT = params->maxRenderDist;

	for(int i = 0; i < params->numSpheres; i++) {
		float t;
		if(raySphere(ray, &spheres[i], &t)) {
			if(t < minT){
				minT = t;
				*type = SPHERE_TYPE_ID;
				*index = i;
			}
		}
	}

	for(int i = 0; i < params->numTris; i++) {
		float t;
		if(rayTriangle(ray, &tris[i], &t)) {
			if(t < minT){
				minT = t;
				*type = SPHERE_TYPE_ID;
				*index = i;
			}
		}
	}

	return minT;
}

float dt(float3 v1, float3 v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float3 doRaytrace(
		struct Ray* ray,
		global struct RenderParams* params,
		global struct Sphere* spheres,
		global struct Triangle* tris,
		global struct PointLight* lights,
		global struct Material* materials, int traceDepth) {

	int intersectObjIndex = -1;
	uint intersectObjType = NULL_TYPE_ID;
	float t = intersect(ray, params, spheres, tris, &intersectObjIndex, &intersectObjType);

	float3 color = (float3){0.0f, 0.0f, 0.0f};

	if(t < params->maxRenderDist) {
		float3 intersectPos = ray->origin + ray->direction*t;
		float3 normal;

		global struct Material* m = &materials[0];

		if(intersectObjType == SPHERE_TYPE_ID) {
			normal = normalize(intersectPos - spheres[intersectObjIndex].position);
			m = &materials[spheres[intersectObjIndex].materialId];
		} else if(intersectObjType == TRIANGLE_TYPE_ID) {
			normal = normalize(
						cross(tris[intersectObjIndex].v2 - tris[intersectObjIndex].v1,
							  tris[intersectObjIndex].v3 - tris[intersectObjIndex].v1));
			m = &materials[tris[intersectObjIndex].materialId];
		}

		// TODO: Accumulate reflected and refracted colors

		for(int i = 0; i < params->numLights; i++) {
			float3 L = lights[i].position - intersectPos;
			float distanceToLight = length(L);
			L = normalize(L);

			struct Ray shadowRay;
			shadowRay.origin = intersectPos + L*0.001;
			shadowRay.direction = L;
			t = intersect(&shadowRay, params, spheres, tris, &intersectObjIndex, &intersectObjType);
			if(t > distanceToLight) {
				color += m->diffuseColor*lights[i].power*max(0.0f, dot(normal, L));
			}
		}
	}

	return clamp(color, 0.0, 1.0);
}

kernel void raytrace(
		global const struct Triangle* triangles,
		global const struct Sphere* spheres,
		global const struct PointLight* lights,
		global const struct Material* materials,
		global const struct RenderParams* params,
		global write_only image2d_t res) {

	struct Ray ray;
	ray.origin = (float3) {0.0f, 0.0f, 0.0f};
	ray.direction = normalize((float3) {(float)get_global_id(0)/(float)get_global_size(0) - 0.5f,
							  	  	  	(float)get_global_id(1)/(float)get_global_size(1) - 0.5f,
							  	  	  	0.5});

	float3 color = doRaytrace(&ray, params, spheres, triangles, lights, materials, 0);
	write_imagef(res, (int2) {get_global_id(0), get_global_id(1)},
				 (float4){color.x, color.y, color.z, 1.0});
}
