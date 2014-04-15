/*
 * geometry_types.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <glm/glm.hpp>

#include <string>

#include "detail/cl_device_context.h"

#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

constexpr cl_float16 mat4ToFloat16(const glm::mat4& mat) {
	return {{
		mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3],
		mat[2][0], mat[2][1], mat[2][2], mat[2][3],
		mat[3][0], mat[3][1], mat[3][2], mat[3][3]
	}};
}

struct Sphere {
	void transform(const glm::mat4& tx);

	cl_float  radius;
	cl_float3 position;
	cl_uint   materialId;
};

struct Triangle {
	void transform(const glm::mat4& tx);

	cl_float3 vp1, vp2, vp3;
	cl_float2 vt1, vt2, vt3;
	cl_float3 normal;
	cl_uint   materialId;
};

struct Quad {
	void transform(const glm::mat4& tx);

	cl_float3 vp1, vp2, vp3, vp4;
	cl_float2 vt1, vt2, vt3, vt4;
	cl_float3 normal;
	cl_uint   materialId;
};

struct PointLight {
	void transform(const glm::mat4& tx);

	cl_float3 position;
	cl_float3 power;
	cl_float  attenuation;
};

template <typename T>
T transform(T& object, const glm::mat4& tx) {
	T t = object;
	t.transform(tx);
	return t;
}

template <typename Iter>
void transform(Iter begin, Iter end, const glm::mat4& tx) {
	for(Iter i = begin; i != end; i++) {
		i->transform(tx);
	}
}

enum BRDFType {
	DIFFUSE,
	PHONG,
	BLINN_PHONG,
	COOK_TORRANCE
};

template <BRDFType BRDF>
struct Material { };

template <>
struct Material<DIFFUSE> {
	cl_float3  reflectivity;
	cl_float3  transmissivity;
	cl_float3  color;
	cl_uint textureId;

	static constexpr const char* name() {
		return "DIFFUSE_BRDF";
	}
};

template <>
struct Material<PHONG> {
	cl_float3  reflectivity;
	cl_float3  transmissivity;
	cl_float3  kd;
	cl_float3  ks;
	cl_float   exp;
	cl_uint    texDiffuse;

	static constexpr const char* name() {
		return "PHONG_BRDF";
	}
};

template <>
struct Material<BLINN_PHONG> {
	cl_float3  reflectivity;
	cl_float3  transmissivity;
	cl_float3  kd;
	cl_float3  ks;
	cl_float   exp;
	cl_uint texDiffuse;

	static constexpr const char* name() {
		return "BLINN_PHONG_BRDF";
	}
};

struct Ray {
	cl_float3 origin;
	cl_float3 direction;
};

#endif /* DATA_TYPES_H_ */
