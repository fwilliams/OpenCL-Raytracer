#include "opencl_ide_fix.h"

#define BLINN_PHONG_BRDF
#include "cl_data_structures.h"
#include "cl_geometry.h"

#define MAX_REFLECTIONS 3

#define RAY_TRI_EPSILON 0.000001

inline float3 computeRadiance(
		float3* position, float3* normal, 
		global struct Material* material, 
		global struct PointLight* lights, 
		global struct Sphere* spheres, 
		global struct Triangle* tris) {
	
	float3 color = (float3){0.0, 0.0, 0.0};
	
	for(int j = 0; j < NUM_LIGHTS; j++) {
		float3 L = lights[j].position - *position;
		float3 pos = *position;
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
			float3 H = normalize(L + -pos);
			color += lights[j].power * (material->kd*max(0.0f, dot(*normal, L)) + material->ks*pow(max(0.0f, dot(*normal, H)), material->exp));
			#elif defined PHONG_BRDF
			color += lights[j].power * (material->kd*max(0.0f, dot(*normal, L)) + material->ks*pow(max(0.0f, dot(*normal, normalize(reflect(L,*normal)))), material->exp));
			#elif defined COOK_TORRANCE_BRDF

			#endif
		}
	}
	return color;
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
	ray.origin = (float3){0.0, 0.0, 0.0};//matrixVectorMultiply(viewMatrix, &((float3) {0.0f, 0.0f, 0.0f}));
	ray.direction = normalize((float3) {
		min(((float)get_global_id(0))/(float)get_global_size(0) - 0.5f, 1.0),
		min(-((float)get_global_id(1))/(float)get_global_size(1) + 0.5f, 1.0),
										-0.5});
	float3 color = doRaytrace(&ray, spheres, triangles, lights, materials, 0);
	
	
	write_imagef(res, (int2) {get_global_id(0), get_global_id(1)},
				 (float4){color.x, color.y, color.z, 1.0});
}