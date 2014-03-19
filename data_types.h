/*
 * geometry_types.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <string>

#ifndef GEOMETRY_TYPES_H_
#define GEOMETRY_TYPES_H_

struct Sphere {
	cl_float radius;
	cl_float3 position;
	cl_uint materialId;
};

struct Triangle {
	cl_float3 v1, v2, v3;
	cl_uint materialId;
};

struct PointLight {
	cl_float3 position;
	cl_float3 power;
};

enum LightModel {
	DIFFUSE,
	PHONG,
	BLINN_PHONG,
	COOK_TORRANCE
};

template <LightModel Model>
struct Material { };

template <>
struct Material<DIFFUSE> {
	cl_float3 reflectivity;
	cl_float3 color;

	static constexpr const char* name() {
		return "DIFFUSE_BRDF";
	}
};

template <>
struct Material<PHONG> {
	cl_float3 reflectivity;
	cl_float3 kd;
	cl_float3 ks;
	cl_float exp;

	static constexpr const char* name() {
		return "PHONG_BRDF";
	}
};

template <>
struct Material<BLINN_PHONG> {
	cl_float3 reflectivity;
	cl_float3 kd;
	cl_float3 ks;
	cl_float exp;

	static constexpr const char* name() {
		return "BLINN_PHONG_BRDF";
	}
};


struct Ray {
	cl_float3 origin;
	cl_float3 direction;
};

#endif /* GEOMETRY_TYPES_H_ */
