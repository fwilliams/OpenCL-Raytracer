#include "opencl_ide_fix.h"

struct Sphere {
	double radius;
	float3 position;
};

struct Triangle {
	float3 vertices[3];
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

kernel void raytrace(global Triangle* triangles, global Sphere* spheres,
		global Material* materials, global float* viewTransform) {

}
