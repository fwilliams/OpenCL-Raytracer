#include "helper_functions.h"

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
		
		for(int i = 0; i < NUM_REFLECTIVE_PASSES; i++) {
			reflectionFactor *= reflectMat->reflectivity;
			
			reflectRay.direction = reflect(normalize(reflectRay.direction), reflectNormal);
			reflectRay.origin += reflectRay.direction * RAY_SURFACE_EPSILON;
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

kernel void single_pass(
		global const struct Triangle* triangles,
		global const struct Sphere* spheres,
		global const struct PointLight* lights,
		global const struct Material* materials,
		global float4* res) {

	struct Ray ray;
	ray.origin = (float3){0.0, 0.0, 0.0};
	ray.direction = normalize((float3) {
		min(((float)get_global_id(0))/(float)get_global_size(0) - 0.5f, 1.0),
		min(-((float)get_global_id(1))/(float)get_global_size(1) + 0.5f, 1.0),
										-0.5});
	
	float3 color = doRaytrace(&ray, spheres, triangles, lights, materials, 0);
	
	res[get_global_id(1) * get_global_size(0) + get_global_id(0)] = (float4){color.x, color.y, color.z, 1.0};
}