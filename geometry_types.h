/*
 * geometry_types.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#ifndef GEOMETRY_TYPES_H_
#define GEOMETRY_TYPES_H_

#define BLINN_PHONG

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
	cl_float3 reflectivity;

#ifdef BLINN_PHONG
	cl_float3 kd;
	cl_float3 ks;
	cl_float exp;
#else
	cl_float3 color;
#endif
};

#endif /* GEOMETRY_TYPES_H_ */
