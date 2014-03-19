/*
 * cl_geometry.h
 *
 *  Created on: Mar 17, 2014
 *      Author: francis
 */

#include "ide_fix.h"
#include "data_structures.h"

#ifndef CL_GEOMETRY_H_
#define CL_GEOMETRY_H_

// TODO: Move all of these somewhere better
#define NULL_TYPE_ID 0
#define SPHERE_TYPE_ID 1
#define TRIANGLE_TYPE_ID 2

#define RAY_SURFACE_EPSILON 0.0001
#define RAY_TRI_EPSILON 0.000001

bool rayTriangle(struct Ray* ray, global struct Triangle* tri, float* outT) {
	float3 e1, e2;
	float3 P, Q, T;
	float det, invDet, u, v;
	float t;

	//Find vectors for two edges sharing v1
	e1 = tri->v2 - tri->v1;
	e2 = tri->v3 - tri->v1;

	//Begin calculating determinant - also used to calculate u parameter
	P = cross(ray->direction, e2);
	det = dot(e1, P);

	//if determinant is near zero, ray lies in plane of triangle
	if(det > -RAY_TRI_EPSILON && det < RAY_TRI_EPSILON) {
		return false;
	}

	invDet = 1.0f / det;

	//calculate distance from V1 to ray origin
	T = ray->origin - tri->v1;

	//Calculate u parameter and test bound
	u = dot(T, P) * invDet;
	if(u < 0.0f || u > 1.0f) {
		//The intersection lies outside of the triangle
		return false;
	}

	//Prepare to test v parameter
	Q = cross(T, e1);

	//Calculate v parameter and test bound
	v = dot(ray->direction, Q) * invDet;
	if(v < 0.0f || u + v  > 1.0f) {
		//The intersection lies outside of the triangle
		return false;
	}

	t = dot(e2, Q) * invDet;

	if(t > RAY_TRI_EPSILON) {
		*outT = t;
		return true;
	}

	return false;
}

bool raySphere(struct Ray* r, global struct Sphere* s, float* t) {
	float3 rayToCenter = s->position - r->origin ;
	float dotProduct = dot(r->direction, rayToCenter);
	float d = dotProduct*dotProduct - dot(rayToCenter,rayToCenter)+s->radius*s->radius;
	float sqrtd = sqrt(d);

	if(d < 0) {
		return false;
	}

	*t = dotProduct - sqrtd;

	if(*t < 0) {
		*t = dotProduct + sqrtd;
		if(*t < 0) {
			return false;
		}
	}

	return true;
}

float intersect(
		struct Ray* ray,
		global struct Sphere* spheres,
		global struct Triangle* tris,
		int* index, uint* type) {
	float minT = MAX_RENDER_DISTANCE;

	for(int i = 0; i < NUM_SPHERES; i++) {
		float t;
		if(raySphere(ray, &spheres[i], &t)) {
			if(t < minT){
				minT = t;
				*type = SPHERE_TYPE_ID;
				*index = i;
			}
		}
	}

	for(int i = 0; i < NUM_TRIANGLES; i++) {
		float t;
		if(rayTriangle(ray, &tris[i], &t)) {
			if(t < minT){
				minT = t;
				*type = TRIANGLE_TYPE_ID;
				*index = i;
			}
		}
	}

	return minT;
}

inline float dt(float3 v1, float3 v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}


float3 reflect(float3 v, float3 n) {
	return v - 2.0f * dot(v, n) * n;
}


inline float3 computeRadiance(
		float3* position, float3* normal,
		global struct Material* material,
		global struct PointLight* lights,
		global struct Sphere* spheres,
		global struct Triangle* tris) {

	float3 color = (float3){0.0, 0.0, 0.0};

	for(int j = 0; j < NUM_LIGHTS; j++) {
		float3 L = lights[j].position - *position;
		float3 pos = *position;
		float distanceToLight = length(L);
		L = normalize(L);

		struct Ray shadowRay;
		shadowRay.origin = *position + L*RAY_SURFACE_EPSILON;
		shadowRay.direction = L;

		int index;
		uint type;
		float st = intersect(&shadowRay, spheres, tris, &index, &type);

		if(st > distanceToLight) {
			#if defined CONST_BRDF
			color += material->color*lights[j].power*max(0.0f, dot(*normal, L));

			#elif defined BLINN_PHONG_BRDF
			float3 H = normalize(L + -pos);
			color += lights[j].power * (material->kd*max(0.0f, dot(*normal, L)) + material->ks*pow(max(0.0f, dot(*normal, H)), material->exp));
			#elif defined PHONG_BRDF
			color += lights[j].power * (material->kd*max(0.0f, dot(*normal, L)) + material->ks*pow(max(0.0f, dot(*normal, normalize(reflect(L,*normal)))), material->exp));
			#elif defined COOK_TORRANCE_BRDF

			#endif
		}
	}
	return color;
}

#endif /* CL_GEOMETRY_H_ */
