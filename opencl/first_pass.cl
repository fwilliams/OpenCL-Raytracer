#include "helper_functions.h"

kernel void first_pass(global struct Ray* rayBuffer,
					   global float3* reflectivityBuffer,
					   global const struct Triangle* triangles,
					   global const struct Sphere* spheres,
					   global const struct PointLight* lights,
					   global const struct Material* materials,
					   global float4* res,
					   float16 viewMatrix) {
	struct Ray ray;
	ray.origin = (float3) {0.0, 0.0, 0.0};
	ray.direction = normalize((float3) {
		min(((float)get_global_id(0))/(float)get_global_size(0) - 0.5f, 1.0),
		min(-((float)get_global_id(1))/(float)get_global_size(1) + 0.5f, 1.0),
										-0.5});
	
	uint i = get_global_size(0)*get_global_id(1) + get_global_id(0);
	
	int intersectObjIndex = -1;
	uint intersectObjType = NULL_TYPE_ID;
	float t = intersect(&ray, spheres, triangles, &intersectObjIndex, &intersectObjType);

	res[i] = (float4){0.0, 0.0, 0.0, 1.0};
	rayBuffer[i].direction = (float3){0.0, 0.0, 0.0};
	
	if(t < MAX_RENDER_DISTANCE) {
		float3 intersectPos = ray.origin + ray.direction*t;
		float3 normal;

		global const struct Material* m = &materials[0];

		if(intersectObjType == SPHERE_TYPE_ID) {
			normal = normalize(intersectPos - spheres[intersectObjIndex].position);
			m = &materials[spheres[intersectObjIndex].materialId];
		} else if(intersectObjType == TRIANGLE_TYPE_ID) {
			normal = normalize(
						cross(triangles[intersectObjIndex].v2 - triangles[intersectObjIndex].v1,
							  triangles[intersectObjIndex].v2 - triangles[intersectObjIndex].v3));
			m = &materials[triangles[intersectObjIndex].materialId];
		}
		
		float3 clr = computeRadiance(&intersectPos, &normal, m, lights, spheres, triangles);
		res[i] += (float4) {clr.x, clr.y, clr.z, 0.0};
		
		if(dot(m->reflectivity, m->reflectivity) != 0.0) {
			reflectivityBuffer[i] = m->reflectivity;
			rayBuffer[i].direction = normalize(reflect(ray.direction, normal));
			rayBuffer[i].origin = intersectPos + RAY_SURFACE_EPSILON*rayBuffer[i].direction;
		}
	}
}