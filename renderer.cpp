/*
 * renderer.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: francis
 */

#include "renderer.h"

#include <fstream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

renderer::renderer(const vector<Sphere>& spheres, const vector<Triangle>& tris,
		const vector<PointLight>& lights, const vector<Material>& materials) :
		numSpheres(spheres.size()), numTris(tris.size()), numLights(
				lights.size()) {

	initOpenCL();
	packBuffers(spheres, tris, lights, materials);
}

void renderer::initOpenCL() {
	vector<cl::Platform> allPlatforms;
	cl::Platform::get(&allPlatforms);

	vector<cl::Device> allDevices;
	allPlatforms[0].getDevices(CL_DEVICE_TYPE_GPU, &allDevices);

	device = allDevices[0];

	ctx = cl::Context( { device });

	cmdQueue = cl::CommandQueue(ctx, allDevices[0]);

	string programFileName("reflectracer.cl");
	program = createProgramFromFile(programFileName);
	raytrace = cl::KernelFunctor(cl::Kernel(program, "raytrace"), cmdQueue,
			cl::NullRange, cl::NDRange(viewportWidth * viewportHeight),
			cl::NullRange);
}

void renderer::packBuffers(const vector<Sphere>& spheres,
		const vector<Triangle>& tris, const vector<PointLight>& lights,
		const vector<Material>& materials) {

	size_t trisByteSize = sizeof(Triangle) * tris.size();
	this->tris = cl::Buffer(ctx, CL_MEM_READ_ONLY, trisByteSize,
			(void*) tris.data(), 0);

	size_t lightByteSize = sizeof(PointLight) * lights.size();
	this->lights = cl::Buffer(ctx, CL_MEM_READ_ONLY, lightByteSize,
			(void*) lights.data(), 0);

	size_t sphereByteSize = sizeof(Sphere) * spheres.size();
	this->spheres = cl::Buffer(ctx, CL_MEM_READ_ONLY, sphereByteSize,
			(void*) spheres.data(), 0);

	size_t materialByteSize = sizeof(Material) * materials.size();
	this->materials = cl::Buffer(ctx, CL_MEM_READ_ONLY, materialByteSize,
			(void*) materials.data(), 0);
}

cl::Program renderer::createProgramFromFile(string& filename) {
	string programString;
	ifstream file;
	file.exceptions(ifstream::badbit);
	file.open(filename.c_str());

	while (!file.eof()) {
		string str;
		getline(file, str);
		programString += str;
		programString.push_back('\n');
	}

	file.close();

	cl::Program::Sources sources;
	sources.push_back( { programString.c_str(), programString.size() });

	cl::Program program(ctx, sources);
	program.build( { device });

	return program;
}

void renderer::renderToTexture() {
	raytrace(tris, spheres, lights, materials, mat4(1.0));
}

