#include "opencl_ide_fix.h"

struct Sphere {
	double radius;
	float3 position;
	uint materialId;
};

struct Triangle {
	float3 vertices[3];
};

struct PointLight {
	float3 position;
	float3 power;
	uint materialId;
};

struct Material {
	float3 reflection;
	float3 refraction;
	float3 absorbtion;
};

kernel void raytrace(global const Triangle* triangles,
		global const Sphere* spheres, global const PointLight* light,
		global Material* materials, global const float* viewTransform) {
}
