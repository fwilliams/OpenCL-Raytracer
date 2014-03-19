/*
 * cl_data_structures.h
 *
 *  Created on: Mar 17, 2014
 *      Author: francis
 */
#include "ide_fix.h"

#ifndef CL_DATA_STRUCTURES_H_
#define CL_DATA_STRUCTURES_H_

struct Material {
	float3 reflectivity;

#if defined CONST_BRDF				// All surfaces lambertian
	float3 color;

#elif defined BLINN_PHONG_BRDF or defined PHONG_BRDF	// Phong lighting (physically implausible)
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

#endif /* CL_DATA_STRUCTURES_H_ */
