/*
 * renderer2.h
 *
 *  Created on: Mar 17, 2014
 *      Author: francis
 */

#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <GL/gl.h>
#include <memory>
#include <vector>
#include <string>

#include "scene.h"

#ifndef RENDERER2_H_
#define RENDERER2_H_

template <cl_device_type DEVICE_TYPE>
struct renderer {
	renderer(std::shared_ptr<Scene<DEVICE_TYPE>> scene,
			unsigned vpWidth, unsigned vpHeight);


	void renderToTexture(GLuint tex, cl_float viewMat[16]);

	void resizeViewport(unsigned vpWidth, unsigned vpHeight);

	void setScene(std::shared_ptr<Scene<DEVICE_TYPE>> newScene) {
		scene = newScene;

		// Create the program
		std::string programFileName("first_pass.cl");
		std::string reflProgFname("reflect.cl");
		std::map<std::string, std::string> defines {
			{"NUM_SPHERES", std::to_string(scene->getNumSpheres())},
			{"NUM_TRIANGLES", std::to_string(scene->getNumTriangles())},
			{"NUM_LIGHTS", std::to_string(scene->getNumPointLights())},
			{"NUM_MATERIALS", std::to_string(scene->getNumMaterials())},
			{"MAX_RENDER_DISTANCE", std::to_string(100.0)}
		};

		cl::Program program2 =
				scene->getCLDeviceContext()->createProgramFromFile(reflProgFname, defines);
		reflectPassKernel = cl::Kernel(program2, "reflect_pass");

		cl::Program program =
				scene->getCLDeviceContext()->createProgramFromFile(programFileName, defines);
		firstPassKernel = cl::Kernel(program, "first_pass");

		setViewportSize(viewportWidth, viewportHeight);
	}

	void setViewportSize(size_t width, size_t height) {
		firstPass = cl::KernelFunctor(firstPassKernel, deviceContext->commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);

		reflectPass = cl::KernelFunctor(reflectPassKernel, deviceContext->commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);
	}

private:
	std::shared_ptr<Scene<CL_DEVICE_TYPE_GPU>> scene;
	std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> deviceContext;

	size_t viewportWidth, viewportHeight;
	cl::Kernel firstPassKernel, reflectPassKernel;
	cl::KernelFunctor firstPass, reflectPass;
	cl::Buffer params, rayBuffer, reflectivityBuffer;
	cl::Buffer resImg;
};

template <cl_device_type DEVICE_TYPE>
renderer<DEVICE_TYPE>::renderer(std::shared_ptr<Scene<DEVICE_TYPE>> scene,
		unsigned vpWidth, unsigned vpHeight) :
				deviceContext(scene->getCLDeviceContext()), viewportWidth(vpWidth), viewportHeight(vpHeight) {
	setScene(scene);
	this->rayBuffer = cl::Buffer(deviceContext->context, CL_MEM_READ_WRITE, sizeof(Ray)*viewportWidth*viewportHeight);
	this->reflectivityBuffer = cl::Buffer(deviceContext->context, CL_MEM_READ_WRITE, sizeof(cl_float3)*viewportWidth*viewportHeight);
	this->resImg = cl::Buffer(deviceContext->context, CL_MEM_READ_WRITE, sizeof(cl_float4)*viewportWidth*viewportHeight);
}

template <cl_device_type DEVICE_TYPE>
void renderer<DEVICE_TYPE>::renderToTexture(GLuint tex, cl_float viewMat[16]) {
	firstPass(rayBuffer,
			reflectivityBuffer,
			scene->getTriangleBuffer(),
			scene->getSphereBuffer(),
			scene->getPointLightBuffer(),
			scene->getMaterialBuffer(),
			resImg);

	for(int i = 0; i < 1; i++) {
		reflectPass(rayBuffer,
				reflectivityBuffer,
				scene->getTriangleBuffer(),
				scene->getSphereBuffer(),
				scene->getPointLightBuffer(),
				scene->getMaterialBuffer(),
				resImg);
	}

	glBindTexture(GL_TEXTURE_2D, tex);

	cl::size_t<3> origin;
	origin.push_back(0);
	origin.push_back(0);
	origin.push_back(0);
	cl::size_t<3> size;
	size.push_back(viewportWidth);
	size.push_back(viewportHeight);
	size.push_back(1);

	cl_float4 pixels[viewportWidth*viewportHeight];

	deviceContext->commandQueue.enqueueReadBuffer(
		resImg, true, 0, viewportWidth*viewportHeight*sizeof(cl_float4), pixels);

	glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);
}

#endif /* RENDERER2_H_ */
