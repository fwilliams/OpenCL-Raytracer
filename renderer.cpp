/*
 * renderer.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: francis
 */

#include "renderer.h"

using namespace std;

renderer::renderer(const vector<Sphere>& spheres, const vector<Triangle>& tris,
		const vector<PointLight>& lights) :
			numSpheres(spheres.size()), numTris(tris.size()), numLights(lights.size()) {
	vector<cl::Platform> allPlatforms;
	vector<cl::Device> allDevices;

	cl::Platform::get(&allPlatforms);

	allPlatforms[0].getDevices(CL_DEVICE_TYPE_GPU, &allDevices);

	ctx = cl::Context( { allDevices[0] });

	cmdQueue = cl::CommandQueue(ctx, allDevices[0]);

	packBuffers(spheres, tris, lights);
}

void renderer::packBuffers(const vector<Sphere>& spheres, const vector<Triangle>& tris,
	const vector<PointLight>& lights) {

	size_t geomByteSize = sizeof(Sphere) * spheres.size()
			+ sizeof(Triangle) * tris.size();
	this->geometry = cl::Buffer(ctx, CL_MEM_READ_ONLY, geomByteSize,
			(void*) tris.data(), 0);

	size_t lightByteSize = sizeof(PointLight) * lights.size();
	this->lights = cl::Buffer(ctx, CL_MEM_READ_ONLY, lightByteSize,
			(void*) lights.data(), 0);

	size_t sphereOffsetSize = sizeof(Triangle) * tris.size();
	size_t sphereByteSize = sizeof(Sphere) * spheres.size();
	cmdQueue.enqueueWriteBuffer(geometry, true, sphereOffsetSize,
			sphereByteSize, (const void*) spheres.data(), nullptr, nullptr);
}

void renderer::renderToTexture() {

}

