/*
 * renderer.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: francis
 */

#include "renderer.h"

#include <fstream>
#include <iostream>


using namespace std;

renderer::renderer(std::shared_ptr<Scene<std::vector>> scene,
		RenderParams& params, unsigned vpWidth, unsigned vpHeight) :
		scene(scene), viewportWidth(vpWidth), viewportHeight(vpHeight) {
	initOpenCL();
	packBuffers(params);
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

void renderer::packBuffers(RenderParams& renderParams) {
	this->params = cl::Buffer(ctx, CL_MEM_READ_ONLY, sizeof(RenderParams));
	cmdQueue.enqueueWriteBuffer(this->params, true, 0, sizeof(RenderParams), &renderParams);

	this->viewMatrix = cl::Buffer(ctx, CL_MEM_READ_ONLY, sizeof(cl_float)*16);

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
		program.build((vector<cl::Device>){ device });
	} catch (cl::Error& e) {
		cerr << string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device))
				<< endl;
	}

	return program;
}

void renderer::renderToTexture(GLuint tex, cl_float viewMat[16]) {
	cmdQueue.enqueueWriteBuffer(this->viewMatrix, true, 0, sizeof(cl_float)*16, viewMat);

	raytrace(scene->getTriangleBuffer(),
			scene->getSphereBuffer(),
			scene->getPointLightBuffer(),
			scene->getMaterialBuffer(),
			params, viewMatrix, resImg);

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

