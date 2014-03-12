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

renderer::renderer(std::shared_ptr<Scene<std::vector, CL_DEVICE_TYPE_GPU>> scene,
		RenderParams& params, unsigned vpWidth, unsigned vpHeight) :
				scene(scene), viewportWidth(vpWidth), viewportHeight(vpHeight) {
	initOpenCL();
	packBuffers(params);
}

void renderer::initOpenCL() {
	string programFileName("reflectracer.cl");
	program = createProgramFromFile(programFileName);
	raytrace = cl::KernelFunctor(cl::Kernel(program, "raytrace"), scene->getCLDeviceContext()->commandQueue,
			cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
			cl::NullRange);
}

void renderer::packBuffers(RenderParams& renderParams) {
	this->params = cl::Buffer(scene->getCLDeviceContext()->context, CL_MEM_READ_ONLY, sizeof(RenderParams));
	scene->getCLDeviceContext()->commandQueue.enqueueWriteBuffer(this->params, true, 0, sizeof(RenderParams), &renderParams);

	this->viewMatrix = cl::Buffer(scene->getCLDeviceContext()->context, CL_MEM_READ_ONLY, sizeof(cl_float)*16);

	resImg = cl::Image2D(scene->getCLDeviceContext()->context, CL_MEM_WRITE_ONLY, cl::ImageFormat(CL_RGBA, CL_FLOAT),
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

	cl::Program program(scene->getCLDeviceContext()->context, sources);
	try {
		program.build((vector<cl::Device>){ scene->getCLDeviceContext()->device });
	} catch (cl::Error& e) {
		cerr << string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(scene->getCLDeviceContext()->device))
				<< endl;
	}

	return program;
}

void renderer::renderToTexture(GLuint tex, cl_float viewMat[16]) {
	scene->getCLDeviceContext()->commandQueue.enqueueWriteBuffer(this->viewMatrix, true, 0, sizeof(cl_float)*16, viewMat);

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

	scene->getCLDeviceContext()->commandQueue.enqueueReadImage(
			resImg, true, origin, size, 0, 0, pixels);

	glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	delete pixels;
}

