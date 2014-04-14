#include "helper_functions.h"

kernel void first_pass(global struct Ray* rayBuffer,
					   global float3* reflectivityBuffer,
					   global const struct Triangle* triangles,
					   global const struct Sphere* spheres,
					   global const struct PointLight* lights,
					   global const struct Material* materials,
					   global const float4* texOffsets,
					   const image2d_t texAtlas,
					   global float4* res,
					   float16 viewMatrix) {
	struct Ray ray;

	ray.origin = mat16vec4(&viewMatrix, &(float4) {0.0, 0.0, 0.0, 1.0}).xyz;
	
	float4 direction = normalize((float4) {
		min(((float)get_global_id(0))/(float)get_global_size(0) - 0.5f, 1.0),
		min(-((float)get_global_id(1))/(float)get_global_size(1) + 0.5f, 1.0),
										-0.5, 0.0});
	ray.direction = mat16vec4(&viewMatrix, &direction).xyz;
	
	uint i = get_global_size(0)*get_global_id(1) + get_global_id(0);
	
	float3 normal;
	float2 texcoord;
	global const struct Material* m;
	float t = intersect(&ray, spheres, triangles, materials, &normal, &texcoord, &m);

	res[i] = (float4){0.0, 0.0, 0.0, 1.0};
	rayBuffer[i].direction = (float3){0.0, 0.0, 0.0};
	
	if(t < MAX_RENDER_DISTANCE) {
		float3 intersectPos = ray.origin + ray.direction*t;
		
		float3 clr = computeRadiance(&intersectPos, &normal, &texcoord, m, lights, spheres, triangles, texOffsets, texAtlas);
		res[i] += (float4) {clr.x, clr.y, clr.z, 0.0};
		
		if(dot(m->reflectivity, m->reflectivity) != 0.0) {
			reflectivityBuffer[i] = m->reflectivity;
			rayBuffer[i].direction = normalize(reflect(ray.direction, normal));
			rayBuffer[i].origin = intersectPos + RAY_SURFACE_EPSILON*rayBuffer[i].direction;
		}
	}
}