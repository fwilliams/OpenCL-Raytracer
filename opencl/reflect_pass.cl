#include "helper_functions.h"

kernel void reflect_pass(global struct Ray* rayBuffer,
					   global float3* reflectivityBuffer,
					   global const struct Triangle* triangles,
					   global const struct Sphere* spheres,
					   global const struct PointLight* lights,
					   global const struct Material* materials,
					   global read_only image2d_t texture,
					   global float4* res) {

	uint i = get_global_size(0)*get_global_id(1) + get_global_id(0);
	
	struct Ray ray = rayBuffer[i];
	rayBuffer[i].direction = (float3){0.0, 0.0, 0.0};
	
	if(dot(ray.direction, ray.direction) != 0.0) {

		float3 normal;
		float2 texcoord;
		global const struct Material* m = &materials[0];
		
		float t = intersect(&ray, spheres, triangles, materials, &normal, &texcoord, &m);
		
		if(t < MAX_RENDER_DISTANCE) {
			float3 intersectPos = ray.origin + ray.direction*t;
	
			float3 clr = reflectivityBuffer[i] * computeRadiance(&intersectPos, &normal, &texcoord, m, lights, spheres, triangles, texture);
			res[i] += (float4) {clr.x, clr.y, clr.z, 0.0};
		
			if(dot(m->reflectivity, m->reflectivity) != 0.0) {
				reflectivityBuffer[i] *= m->reflectivity;
				rayBuffer[i].direction = normalize(reflect(ray.direction, normal));
				rayBuffer[i].origin = intersectPos + RAY_SURFACE_EPSILON*rayBuffer[i].direction;
			}
		}
	}
}