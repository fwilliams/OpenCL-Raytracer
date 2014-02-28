#include "opencl_ide_fix.h"

// TODO: Make this into a kernel parameter
constant uint NULL_TYPE_ID = 0;
constant uint SPHERE_TYPE_ID = 1;

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

struct Material {
	float3 reflection;
	float3 refraction;
	float3 diffuseColor;
};

struct Ray {
	float3 origin;
	float3 direction;
};

// TODO: #define these for increases performance
struct RenderParams {
	float maxRenderDist;
	uint numTris;
	uint numSpheres;
	uint numLights;
};

bool raySphere(global struct Sphere* s, struct Ray* r, float* t) {
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

float intersect(struct Ray* ray, global struct RenderParams* params,
				global struct Sphere* spheres, int* index, uint* type) {
	float minT = params->maxRenderDist;

	for(int i = 0; i < params->numSpheres; i++) {
		float t;
		if(raySphere(&spheres[i], ray, &t)) {
			if(t < minT){
				minT = t;
				*type = SPHERE_TYPE_ID;
				*index = i;
			}
		}
	}

	return minT;
}

float dt(float3 v1, float3 v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float3 doRaytrace(struct Ray* ray,
				global struct RenderParams* params,
				global struct Sphere* spheres,
				global struct PointLight* lights,
				global struct Material* materials, int traceDepth) {

	int intersectObjIndex = -1;
	uint intersectObjType = NULL_TYPE_ID;
	float t = intersect(ray, params, spheres, &intersectObjIndex, &intersectObjType);

	float3 color = (float3){0.0f, 0.0f, 0.0f};

	if(t < params->maxRenderDist) {
		float3 intersectPos = ray->origin + ray->direction*t;
		float3 normal;

		global struct Material* m = &materials[0];

		if(intersectObjType == SPHERE_TYPE_ID){
			normal = normalize(intersectPos - spheres[intersectObjIndex].position);
			m = &materials[spheres[intersectObjIndex].materialId];
		}

		// TODO: Accumulate reflected and refracted colors

		for(int i = 0; i < params->numLights; i++){
			float3 L = lights[i].position - intersectPos;
			float distanceToLight = length(L);
			L = normalize(L);

			struct Ray shadowRay;
			shadowRay.origin = intersectPos + L*0.001;
			shadowRay.direction = L;
			t = intersect(&shadowRay, params, spheres, &intersectObjIndex, &intersectObjType);
			if(t > distanceToLight) {
				color += m->diffuseColor*lights[i].power*max(0.0f, dot(normal, L));
			}
		}
	}

	return clamp(color, 0.0, 1.0);
}

kernel void raytrace(
		global const struct Triangle* triangles,
		global const struct Sphere* spheres,
		global const struct PointLight* lights,
		global const struct Material* materials,
		global const struct RenderParams* params,
		global write_only image2d_t res) {

	struct Ray ray;
	ray.origin = (float3) {0.0f, 0.0f, 0.0f};
	ray.direction = normalize((float3) {(float)get_global_id(0)/(float)get_global_size(0) - 0.5f,
							  	  	  	(float)get_global_id(1)/(float)get_global_size(1) - 0.5f,
							  	  	  	0.5});

	float3 color = doRaytrace(&ray, params, spheres, lights, materials, 0);
	write_imagef(res, (int2) {get_global_id(0), get_global_id(1)},
				 (float4){color.x, color.y, color.z, 1.0});
}
