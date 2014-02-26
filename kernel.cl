#include "opencl_ide_fix.h"

constant int kAntiAliasingSamples  = 2;
constant int kMaxTraceDepth = 2;
constant float kMaxRenderDist = 1000.0f;

struct Material {
	/* 0 - Standard diffuse color, 1 - Compute 'Chessboard' texture */
	int computeColorType;
	float4 color;
	float reflectivity;
	float refractivity;
};

struct Material createMaterial() {
	struct Material m;
	m.color = (float4)(1,1,1,1);
	m.computeColorType = 0;
	m.reflectivity = 0;
	m.refractivity = 0;
	return m;
}

struct Sphere {
	struct Material* m;
	float3 pos;
	float radius;
};

struct Plane {
	struct Material* m;
	float3 normal;
	float3 point;
};

struct Ray {
	float3 origin;
	float3 dir;
};

struct Light {
	float3 pos;
	float3 dir;
	bool directional;
	float4 color;
};


struct Scene {
	struct Sphere spheres[10];
	int spheresCount;

	struct Plane planes[10];
	int planesCount;

	struct Light lights[10];
	int lightsCount;

	struct Material standardMaterial;
};

float3 reflect(float3 V, float3 N) {
	return V - 2.0f * dot( V, N ) * N;
}

float3 refract(float3 V, float3 N, float refrIndex) {
    float cosI = -dot( N, V );
    float cosT2 = 1.0f - refrIndex * refrIndex * (1.0f - cosI * cosI);
	return (refrIndex * V) + (refrIndex * cosI - sqrt( cosT2 )) * N;
}

bool raySphere(struct Sphere* s, struct Ray* r, float* t) {
	float3 rayToCenter = s->pos - r->origin ;
	float dotProduct = dot(r->dir,rayToCenter);
	float d = dotProduct*dotProduct - dot(rayToCenter,rayToCenter)+s->radius*s->radius;

	if ( d < 0)
		return false;

	*t = (dotProduct - sqrt(d) );

	if ( *t < 0 ){
		*t = (dotProduct + sqrt(d) ) ;
		if ( *t < 0){
			return false;
		}
	}

	return true;
}

bool rayPlane(struct Plane* p, struct Ray* r, float* t) {
	float dotProduct = dot(r->dir,p->normal);
	if ( dotProduct == 0){
		return false;
	}
	*t = dot(p->normal,p->point-r->origin) / dotProduct ;

	return *t >= 0;
}

float intersect(struct Ray* ray, struct Scene* scene, void** object, int* type) {
	float minT = kMaxRenderDist;

	for(int i = 0; i < scene->spheresCount; i++){
		float t;
		if ( raySphere( &scene->spheres[i], ray, &t ) ){
			if ( t < minT ){
				minT = t;
				*type = 1;
				*object = &scene->spheres[i];
			}
		}
	}

	for(int i = 0; i < scene->planesCount; i++){
		float t;
		if ( rayPlane( &scene->planes[i], ray, &t ) ){
			if ( t < minT ){
				minT = t;
				*type = 2;
				*object = &scene->planes[i];
			}
		}
	}

	return minT;
}

float4 raytrace(struct Ray* ray, struct Scene* scene,int traceDepth) {
	void* intersectObj = 0;
	int intersectObjType = 0;
	float t = intersect( ray, scene, &intersectObj, &intersectObjType);

	float4 color = (float4)(0,0,0,0);
	if ( t < kMaxRenderDist ){
		float3 intersectPos = ray->origin+ray->dir*t ;
		float3 normal;

		struct Material* m = 0;

		if ( intersectObjType == 1 ){
			normal = normalize(intersectPos-((struct Sphere*)intersectObj)->pos);
			m = ((struct Sphere*)intersectObj)->m;
		}
		else if (intersectObjType == 2 ){
			normal = ((struct Plane*)intersectObj)->normal;
			m = ((struct Plane*)intersectObj)->m;
		}

		if ( !m ){
			m = &scene->standardMaterial;
		}

		float4 diffuseColor = m->color;

		if ( m->computeColorType == 1){
			if ( (int)(intersectPos.x/5.0f) % 2 == 0 ){
				if ( (int)(intersectPos.z/5.0f) % 2 == 0 ){
					diffuseColor = (float4)(0,0,0,0);
				}
			}
			else{
				if ( (int)(intersectPos.z/5.0f) % 2 != 0 ){
					diffuseColor = (float4)(0,0,0,0);
				}
			}
		}
		if ( traceDepth < kMaxTraceDepth && m->reflectivity > 0 ){
				struct Ray reflectRay;
				float3 R = reflect(ray->dir, normal);
				reflectRay.origin = intersectPos + R*0.001;
				reflectRay.dir    = R;
//				diffuseColor += m->reflectivity*raytrace(&reflectRay, scene, traceDepth+1);
		}

		if ( traceDepth < kMaxTraceDepth && m->refractivity > 0 ){
				struct Ray refractRay;
				float3 R = refract(ray->dir, normal, 0.6);
				if ( dot(R,normal) < 0 ){
					refractRay.origin = intersectPos + R*0.001;
					refractRay.dir    = R;
//					diffuseColor = m->refractivity*raytrace(&refractRay, scene, traceDepth+1);
				}
		}

		for(int i = 0; i < scene->lightsCount; i++){
			float3 L = scene->lights[i].dir;
			float lightDist = kMaxRenderDist;
			if ( !scene->lights[i].directional ){
				L = scene->lights[i].pos - intersectPos ;
				lightDist = length(L);
				L = normalize(L);
			}

			float pointLit = 1;
			struct Ray shadowRay;
			shadowRay.origin = intersectPos + L*0.001;
			shadowRay.dir = L;
			t = intersect( &shadowRay, scene, &intersectObj, &intersectObjType);
			if ( t < lightDist ){
				pointLit = 0;
			}
			color += pointLit*diffuseColor*scene->lights[i].color*max(0.0f,dot(normal, L));
		}
	}
	return clamp(color, 0.0, 1.0);
}

float3 matrixVectorMultiply(global float* matrix, float3* vector) {
	float3 result;
	result.x = matrix[0]*((*vector).x)+matrix[4]*((*vector).y)+matrix[8]*((*vector).z)+matrix[12];
	result.y = matrix[1]*((*vector).x)+matrix[5]*((*vector).y)+matrix[9]*((*vector).z)+matrix[13];
	result.z = matrix[2]*((*vector).x)+matrix[6]*((*vector).y)+matrix[10]*((*vector).z)+matrix[14];
	return result;
}

kernel void main(global float4 *dst, uint width, uint height, global float* viewTransform, global float* worldTransforms) {
	struct Scene scene;

	scene.standardMaterial = createMaterial();
	scene.standardMaterial.reflectivity = 0;
	scene.standardMaterial.computeColorType = 1;

	struct Material floorMaterial = createMaterial();
	floorMaterial.reflectivity = 0.5;
	floorMaterial.computeColorType = 1;

	struct Material ballMaterial1 = createMaterial();
	ballMaterial1.reflectivity = 1;
	ballMaterial1.color = (float4)(1,0,0,1);
	struct Material ballMaterial2 = createMaterial();
	ballMaterial2.reflectivity = 1;
	ballMaterial2.color = (float4)(0,0,1,1);
	struct Material ballMaterial3 = createMaterial();
	ballMaterial3.reflectivity = 1;
	ballMaterial3.color = (float4)(1,1,1,1);

	struct Material refractMaterial = createMaterial();
	refractMaterial.refractivity = 1;

	scene.spheresCount = 2;
	scene.spheres[0].pos = (float3)(0,0,0);
	scene.spheres[0].radius = 3;
	scene.spheres[0].m = &ballMaterial1;
	scene.spheres[1].pos = (float3)(0,0,-0);
	scene.spheres[1].radius = 3;
	scene.spheres[1].m = &ballMaterial2;

	scene.planesCount = 1;
	scene.planes[0].point = (float3)(0,-5,0);
	scene.planes[0].normal = (float3)(0,1,0);
	scene.planes[0].m      = &floorMaterial;
	scene.planes[1].point = (float3)(0,40,0);
	scene.planes[1].normal = normalize((float3)(0,-1,0));
	scene.planes[2].point = (float3)(-40,-5,0);
	scene.planes[2].normal = (float3)(1,1,0);
	scene.planes[3].point = (float3)(40,-5,0);
	scene.planes[3].normal = normalize((float3)(-1,1,0));

	scene.planes[4].point = (float3)(0,0,0);
	scene.planes[4].normal = normalize((float3)(0,0,-1));
	scene.planes[4].m = &refractMaterial;

	scene.lightsCount = 2;
	scene.lights[0].pos = (float3)(0,30,-20);
	scene.lights[0].directional = false;
	scene.lights[0].color = (float4)(1,1,1,1);
	scene.lights[1].pos = (float3)(0,30,20);
	scene.lights[1].dir = normalize((float3)(0,1,1));
	scene.lights[1].directional = false;
	scene.lights[1].color = (float4)(1,1,1,1);

	scene.spheres[0].pos = matrixVectorMultiply(worldTransforms, &scene.spheres[0].pos);
	scene.spheres[1].pos = matrixVectorMultiply(worldTransforms+16, &scene.spheres[1].pos);

	float dx = 1.0f / (float)(width);
	float dy = 1.0f / (float)(height);
	float aspect = (float)(width) / (float)(height);

	dst[get_global_id(0)] = (float4)(0,0,0,0);
	for(int i = 0; i < kAntiAliasingSamples; i++){
		for(int j = 0; j < kAntiAliasingSamples; j++){
				float x = (float)(get_global_id(0) % width) / (float)(width) + dx*i/kAntiAliasingSamples;
				float y = (float)(get_global_id(0) / width) / (float)(height) + dy*j/kAntiAliasingSamples;

				x = (x -0.5f)*aspect;
				y = y -0.5f;

				struct Ray r;
				r.origin = matrixVectorMultiply(viewTransform, &((float3){0.0, 0.0, -1.0}));
				r.dir    = normalize(matrixVectorMultiply(viewTransform, &((float3){x, y, 0.0})) - r.origin);
				float4 color = raytrace(&r, &scene, 0);
				dst[get_global_id(0)] += color / (kAntiAliasingSamples*kAntiAliasingSamples) ;
		}
	}
}
