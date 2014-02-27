#include "opencl_ide_fix.h"

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
	float3 absorbtion;
};

kernel void raytrace(
		global struct Triangle* triangles,
		global struct Sphere* spheres,
		global struct PointLight* lights,
		global struct Material* materials,
		global write_only image2d_t res) {

//	write_imagef(res, (int2 ) { get_global_id(0), get_global_id(1) },
//			(float4) (get_global_id(0)/640.0, get_global_id(1)/480.0, 0.5, 1.0));

	float3 color = materials[0].reflection;

	write_imagef(res, (int2) { get_global_id(0), get_global_id(1) },
			(float4) { color.x, color.y, color.z, 1.0 });
}
