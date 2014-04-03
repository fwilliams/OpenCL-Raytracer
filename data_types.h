/*
 * geometry_types.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <string>
#include <glm/glm.hpp>

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
	cl_float  radius;
	cl_float3 position;
	cl_uint   materialId;
};

struct Triangle {
	cl_float3 v1, v2, v3;
	cl_float2 vt1, vt2, vt3;
	cl_float3 normal;
	cl_uint   materialId;
};

struct PointLight {
	cl_float3 position;
	cl_float3 power;
	cl_float  attenuation;
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
	cl_float3 transmissivity;
	cl_float3 color;

	static constexpr const char* name() {
		return "DIFFUSE_BRDF";
	}
};

template <>
struct Material<PHONG> {
	cl_float3 reflectivity;
	cl_float3 transmissivity;
	cl_float3 kd;
	cl_float3 ks;
	cl_float  exp;

	static constexpr const char* name() {
		return "PHONG_BRDF";
	}
};

template <>
struct Material<BLINN_PHONG> {
	cl_float3 reflectivity;
	cl_float3 transmissivity;
	cl_float3 kd;
	cl_float3 ks;
	cl_float  exp;

	static constexpr const char* name() {
		return "BLINN_PHONG_BRDF";
	}
};


struct Ray {
	cl_float3 origin;
	cl_float3 direction;
};


template <typename T>
void transform(T& object, const glm::mat4& tx);

template <>
void transform<Sphere>(Sphere& sphere, const glm::mat4& tx) {
	glm::vec4 pos = tx * glm::vec4(sphere.position.s[0], sphere.position.s[1], sphere.position.s[2], 1.0);
	sphere.position = {{pos.x, pos.y, pos.z}};
}

template <>
void transform<Triangle>(Triangle& tri, const glm::mat4& tx) {
	glm::vec4 v;

	v = tx * glm::vec4(tri.v1.s[0], tri.v1.s[1], tri.v1.s[2], 1.0);
	tri.v1 = {{v.x, v.y, v.z}};

	v = tx * glm::vec4(tri.v2.s[0], tri.v2.s[1], tri.v2.s[2], 1.0);
	tri.v2 = {{v.x, v.y, v.z}};

	v = tx * glm::vec4(tri.v3.s[0], tri.v3.s[1], tri.v3.s[2], 1.0);
	tri.v3 = {{v.x, v.y, v.z}};

	v = tx * glm::vec4(tri.normal.s[0], tri.normal.s[1], tri.normal.s[2], 0.0);
	tri.normal = {{v.x, v.y, v.z}};
}

template <>
void transform<PointLight>(PointLight& light, const glm::mat4& tx) {
	glm::vec4 pos = tx * glm::vec4(light.position.s[0], light.position.s[1], light.position.s[2], 1.0);
	light.position = {{pos.x, pos.y, pos.z}};
}

template <typename T>
struct TransformFunctor {
	TransformFunctor(const glm::mat4& tx) : tx(tx) {}

	T operator()(T val) {
		transform<T>(val, tx);
		return val;
	}

private:
	glm::mat4 tx;
};

template <typename T>
struct MutateTransformFunctor {
	MutateTransformFunctor(const glm::mat4& tx) : tx(tx) {}

	void operator()(T& val) {
		transform<T>(val, tx);
	}

private:
	glm::mat4 tx;
};
#endif /* DATA_TYPES_H_ */
