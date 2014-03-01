/*
 * renderer.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: francis
 */

#include "renderer.h"

#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

renderer::renderer(const vector<Sphere>& spheres, const vector<Triangle>& tris,
		const vector<PointLight>& lights, const vector<Material>& materials,
		RenderParams& params, unsigned vpWidth, unsigned vpHeight) :
		viewportWidth(vpWidth), viewportHeight(vpHeight), numSpheres(spheres.size()), numTris(
				tris.size()), numLights(lights.size()) {
	initOpenCL();
	packBuffers(spheres, tris, lights, materials, params);
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
			cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
			cl::NullRange);
}

void renderer::packBuffers(const vector<Sphere>& spheres,
		const vector<Triangle>& tris, const vector<PointLight>& lights,
		const vector<Material>& mats, RenderParams& renderParams) {

	size_t trisByteSize = sizeof(Triangle) * tris.size();
	this->tris = cl::Buffer(ctx, CL_MEM_READ_ONLY, trisByteSize);
	cmdQueue.enqueueWriteBuffer(this->tris, true, 0, trisByteSize,tris.data());

	size_t lightByteSize = sizeof(PointLight) * lights.size();
	this->lights = cl::Buffer(ctx, CL_MEM_READ_ONLY, lightByteSize);
	cmdQueue.enqueueWriteBuffer(this->lights, true, 0, lightByteSize, lights.data());

	size_t sphereByteSize = sizeof(Sphere) * spheres.size();
	this->spheres = cl::Buffer(ctx, CL_MEM_READ_ONLY, sphereByteSize);
	cmdQueue.enqueueWriteBuffer(this->spheres, true, 0, sphereByteSize, spheres.data());

	size_t materialByteSize = sizeof(Material) * mats.size();
	this->materials = cl::Buffer(ctx, CL_MEM_READ_ONLY, materialByteSize);
	cmdQueue.enqueueWriteBuffer(this->materials, true, 0, materialByteSize, mats.data());

	this->params = cl::Buffer(ctx, CL_MEM_READ_ONLY, sizeof(RenderParams));
	cmdQueue.enqueueWriteBuffer(this->params, true, 0, sizeof(RenderParams), &renderParams);

	resImg = cl::Image2D(ctx, CL_MEM_WRITE_ONLY, cl::ImageFormat(CL_RGBA, CL_FLOAT),
							viewportWidth, viewportHeight, 0);

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
	sources.push_back({ programString.c_str(), programString.size() });

	cl::Program program(ctx, sources);
	try {
		program.build({ device });
	} catch (cl::Error& e) {
		cerr << string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device))
				<< endl;
	}

	return program;
}

void renderer::renderToTexture(GLuint tex) {
	raytrace(tris, spheres, lights, materials, params, resImg);
	glBindTexture(GL_TEXTURE_2D, tex);

	cl::size_t<3> origin;
	origin.push_back(0);
	origin.push_back(0);
	origin.push_back(0);
	cl::size_t<3> size;
	size.push_back(viewportWidth);
	size.push_back(viewportHeight);
	size.push_back(1);

	cl_float4* pixels = new cl_float4[viewportWidth*viewportHeight];

	cmdQueue.enqueueReadImage(
			resImg, true, origin, size, 0, 0, pixels);

	glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	delete pixels;
}

