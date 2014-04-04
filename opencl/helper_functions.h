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

#define RAY_SURFACE_EPSILON 0.001
#define RAY_TRI_EPSILON 0.0001
//#define BACKFACE_CULL

bool rayTriangle(struct Ray* ray, global const struct Triangle* tri, float* outT, float3* outN, float2* outTexCoord) {
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

#ifdef BACKFACE_CULL
	if(det < 0.0) {
		return false;
	}
#else
	//if determinant is near zero, ray lies in plane of triangle
	if(det > -RAY_TRI_EPSILON && det < RAY_TRI_EPSILON) {
		return false;
	}
#endif
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
		*outTexCoord = v*tri->vt3 + (1.0-v)*(u*tri->vt2 + (1.0-u)*tri->vt1);

		if(dot(tri->normal, tri->normal) == 0.0) {
			*outN = normalize(cross(e1, e2));
		} else {
			*outN = normalize(tri->normal);
		}
		return true;
	}

	return false;
}

bool raySphere(struct Ray* r, global const struct Sphere* s, float* outT, float3* outN) {
	float3 rayToCenter = s->position - r->origin ;
	float dotProduct = dot(r->direction, rayToCenter);
	float d = dotProduct*dotProduct - dot(rayToCenter,rayToCenter)+s->radius*s->radius;
	float sqrtd = sqrt(d);

	if(d < 0) {
		return false;
	}

	*outT = dotProduct - sqrtd;

	if(*outT < 0) {
		*outT = dotProduct + sqrtd;
		if(*outT < 0) {
			return false;
		}
	}

	*outN = normalize((r->origin+*outT*r->direction) - s->position);
	return true;
}

float intersect(
		struct Ray* ray,
		global const struct Sphere* spheres,
		global const struct Triangle* tris,
		global const struct Material* mats,
		float3* outN, float2* outTexCoord,
		global const struct Material** outMat) {
	float minT = MAX_RENDER_DISTANCE;
	float3 n;
	float2 tc;

	for(int i = 0; i < NUM_SPHERES; i++) {
		float t;
		if(raySphere(ray, &spheres[i], &t, &n)) {
			if(t < minT){
				minT = t;
				*outMat = &mats[spheres[i].materialId];
				*outN = n;
				*outTexCoord = (float2) {0.0, 0.0};
			}
		}
	}

	for(int i = 0; i < NUM_TRIANGLES; i++) {
		float t;
		if(rayTriangle(ray, &tris[i], &t, &n, &tc)) {
			if(t < minT){
				minT = t;
				*outMat = &mats[tris[i].materialId];
				*outN = n;
				*outTexCoord = tc;
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

float3 computeRadiance(
		float3* position, float3* normal, float2* texcoord,
		global const struct Material* material,
		global const struct PointLight* lights,
		global const struct Sphere* spheres,
		global const struct Triangle* tris,
		global image2d_t texture) {

	float3 color = (float3){0.0, 0.0, 0.0};

	for(int j = 0; j < NUM_LIGHTS; j++) {
		float3 L = lights[j].position - *position;
		float distanceToLight = length(L);
		L = normalize(L);

		struct Ray shadowRay;
		shadowRay.origin = *position + L*RAY_SURFACE_EPSILON;
		shadowRay.direction = L;

		int index;
		uint type;
		float3 n;
		float2 tc;
		global const struct Material* mat;

		float st = intersect(&shadowRay, spheres, tris, 0, &n, &tc, &mat);

		if(st > distanceToLight) {
			float attenuation = (1.0/(1.0 + lights[j].attenuation*distanceToLight));

			#if defined DIFFUSE_BRDF
			color += attenuation*material->color*lights[j].power*max(0.0f, dot(*normal, L));

			#elif defined BLINN_PHONG_BRDF
			float3 H = normalize(L + -*position);
			color += attenuation *
					 (lights[j].power * (material->kd*max(0.0f, dot(*normal, L)) +
					  material->ks*pow(max(0.0f, dot(*normal, H)), material->exp)));
			#elif defined PHONG_BRDF
			color += attenuation *
					 (lights[j].power * (material->kd*max(0.0f, dot(*normal, L)) +
					  material->ks*pow(max(0.0f, dot(*normal, normalize(reflect(L,*normal)))), material->exp)));
			#elif defined COOK_TORRANCE_BRDF

			#endif
		}
	}
	if(material->textureId != 0) {
		const sampler_t samp = CLK_NORMALIZED_COORDS_TRUE;
		float4 texcolor = read_imagef(texture, samp, *texcoord);
		color *= (float3){texcolor.x, texcolor.y, texcolor.z};
	}
	return color;
}

#endif /* CL_GEOMETRY_H_ */
