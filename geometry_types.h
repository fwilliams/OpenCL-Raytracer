/*
 * geometry_types.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

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

struct Material {
	cl_float3 reflection;
	cl_float3 refraction;
	cl_float3 diffuseColor;;
};

#endif /* GEOMETRY_TYPES_H_ */
