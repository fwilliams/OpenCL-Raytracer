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

float16 mat16mat16(float16* a, float16* b) {
	float16 ret;
	ret.s0 = a->s0 *  b->s0 + a->s4 * b->s1 + a->s8 * b->s2 + a->sc * b->s3;
	ret.s1 = a->s1 *  b->s0 + a->s5 * b->s1 + a->s9 * b->s2 + a->sd * b->s3;
	ret.s2 = a->s2 *  b->s0 + a->s6 * b->s1 + a->sa * b->s2 + a->se * b->s3;
	ret.s3 = a->s3 *  b->s0 + a->s7 * b->s1 + a->sb * b->s2 + a->sf * b->s3;

	ret.s4 = a->s0 *  b->s4 + a->s4 * b->s5 + a->s8 * b->s6 + a->sc * b->s7;
	ret.s5 = a->s1 *  b->s4 + a->s5 * b->s5 + a->s9 * b->s6 + a->sd * b->s7;
	ret.s6 = a->s2 *  b->s4 + a->s6 * b->s5 + a->sa * b->s6 + a->se * b->s7;
	ret.s7 = a->s3 *  b->s4 + a->s7 * b->s5 + a->sb * b->s6 + a->sf * b->s7;

	ret.s8 = a->s0 *  b->s8 + a->s4 * b->s9 + a->s8 * b->sa + a->sc * b->sb;
	ret.s9 = a->s1 *  b->s8 + a->s5 * b->s9 + a->s9 * b->sa + a->sd * b->sb;
	ret.sa = a->s2 *  b->s8 + a->s6 * b->s9 + a->sa * b->sa + a->se * b->sb;
	ret.sb = a->s3 *  b->s8 + a->s7 * b->s9 + a->sb * b->sa + a->sf * b->sb;

	ret.sc = a->s0 *  b->sc + a->s4 * b->sd + a->s8 * b->se + a->sc * b->sf;
	ret.sd = a->s1 *  b->sc + a->s5 * b->sd + a->s9 * b->se + a->sd * b->sf;
	ret.se = a->s2 *  b->sc + a->s6 * b->sd + a->sa * b->se + a->se * b->sf;
	ret.sf = a->s3 *  b->sc + a->s7 * b->sd + a->sb * b->se + a->sf * b->sf;

	return ret;
}

float4 mat16vec4(float16* a, float4* b) {
	float4 ret;
	ret.x = a->s0 * b->x + a->s4 * b->y + a->s8 * b->z + a->sc * b->w;
	ret.y = a->s1 * b->x + a->s5 * b->y + a->s9 * b->z + a->sd * b->w;
	ret.z = a->s2 * b->x + a->s6 * b->y + a->sa * b->z + a->se * b->w;
	ret.w = a->s3 * b->x + a->s7 * b->y + a->sb * b->z + a->sf * b->w;
	return ret;
}

bool rayTriangle(struct Ray* ray, struct Triangle tri, float* outT, float3* outN, float2* outTexCoord) {
	float3 e1, e2;
	float3 P, Q, T;
	float det, invDet, u, v;
	float t;

	//Find vectors for two edges sharing v1
	e1 = tri.vp2 - tri.vp1;
	e2 = tri.vp3 - tri.vp1;

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
	T = ray->origin - tri.vp1;

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
		*outTexCoord = (1.0-u-v)*tri.vt1 + u*tri.vt2 + v*tri.vt3;
		*outN = normalize(tri.normal);
		return true;
	}

	return false;
}

bool raySphere(struct Ray* r, struct Sphere s, float* outT, float3* outN) {
	float3 rayToCenter = s.position - r->origin ;
	float dotProduct = dot(r->direction, rayToCenter);
	float d = dotProduct * dotProduct - dot(rayToCenter,rayToCenter) + s.radius * s.radius;
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

	*outN = normalize((r->origin+*outT * r->direction) - s.position);
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
		if(raySphere(ray, spheres[i], &t, &n)) {
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
		if(rayTriangle(ray, tris[i], &t, &n, &tc)) {
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

float intersectTest(
		struct Ray* ray,
		global const struct Sphere* spheres,
		global const struct Triangle* tris) {
	float minT = MAX_RENDER_DISTANCE;
	float3 n;
	float2 tc;

	for(int i = 0; i < NUM_SPHERES; i++) {
		float t;
		if(raySphere(ray, spheres[i], &t, &n)) {
			if(t < minT){
				minT = t;
			}
		}
	}

	for(int i = 0; i < NUM_TRIANGLES; i++) {
		float t;
		if(rayTriangle(ray, tris[i], &t, &n, &tc)) {
			if(t < minT){
				minT = t;
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
		global const float4* texOffsets,
		const image2d_t texAtlas) {

	float3 color = (float3){0.0, 0.0, 0.0};

	float3 texColor = (float3) {1.0, 1.0, 1.0};

	if(material->textureId != 0) {
		const sampler_t samp = CLK_FILTER_LINEAR;
		float4 texOffset = texOffsets[material->textureId - 1];
		float2 texcoordTx = ((*texcoord) * texOffset.zw) + texOffset.xy;
		texColor = read_imagef(texAtlas, samp, texcoordTx).xyz;
	}

	for(int j = 0; j < NUM_LIGHTS; j++) {
		float3 L = lights[j].position - *position;
		float distanceToLight = length(L);
		L = normalize(L);

		struct Ray shadowRay;
		shadowRay.origin = *position + L * RAY_SURFACE_EPSILON;
		shadowRay.direction = L;

		float st = intersectTest(&shadowRay, spheres, tris);

		if(st > distanceToLight) {
			float attenuation = (1.0/(1.0 + lights[j].attenuation*distanceToLight));

			#if defined DIFFUSE_BRDF
			color += attenuation*texColor*material->color*lights[j].power*max(0.0f, dot(*normal, L));

			#elif defined BLINN_PHONG_BRDF

			float3 H = normalize(L + -*position);
			color += attenuation *
					 (lights[j].power * (texColor*material->kd*max(0.0f, dot(*normal, L)) +
					  material->ks*pow(max(0.0f, dot(*normal, H)), material->exp)));
			#elif defined PHONG_BRDF
			color += attenuation *
					 (lights[j].power * (texColor*material->kd*max(0.0f, dot(*normal, L)) +
					  material->ks*pow(max(0.0f, dot(*normal, normalize(reflect(L,*normal)))), material->exp)));
			#elif defined COOK_TORRANCE_BRDF

			#endif
		}
	}
	return color;
}

#endif /* CL_GEOMETRY_H_ */
