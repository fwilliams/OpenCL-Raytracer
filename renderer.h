/*
 * renderer.h
 *
 *  Created on: Feb 24, 2014
 *      Author: francis
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <GL/gl.h>
#include <memory>
#include <vector>
#include <string>

#include "scene.h"

template <cl_device_type DEVICE_TYPE>
struct renderer {
	renderer(std::shared_ptr<Scene<std::vector, DEVICE_TYPE>> scene,
			unsigned vpWidth, unsigned vpHeight);


	void renderToTexture(GLuint tex, cl_float viewMat[16]);

	void resizeViewport(unsigned vpWidth, unsigned vpHeight);

	void setScene(std::shared_ptr<Scene<std::vector, DEVICE_TYPE>> newScene) {
		scene = newScene;

		// Create the program
		std::string programFileName("reflectracer.cl");
		std::map<std::string, std::string> defines {
			{"NUM_SPHERES", std::to_string(scene->getNumSpheres())},
			{"NUM_TRIANGLES", std::to_string(scene->getNumTriangles())},
			{"NUM_LIGHTS", std::to_string(scene->getNumPointLights())},
			{"NUM_MATERIALS", std::to_string(scene->getNumMaterials())},
			{"MAX_RENDER_DISTANCE", std::to_string(100.0)}
		};

		cl::Program program = scene->getCLDeviceContext()->createProgramFromFile(programFileName, defines);
		raytrace = cl::KernelFunctor(cl::Kernel(program, "raytrace"), deviceContext->commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);
	}

private:
	std::shared_ptr<Scene<std::vector, CL_DEVICE_TYPE_GPU>> scene;
	std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> deviceContext;

	size_t viewportWidth, viewportHeight;

	cl::KernelFunctor raytrace;
	cl::Buffer params, viewMatrix;
	cl::Image2D resImg;
};

template <cl_device_type DEVICE_TYPE>
renderer<DEVICE_TYPE>::renderer(std::shared_ptr<Scene<std::vector, DEVICE_TYPE>> scene,
		unsigned vpWidth, unsigned vpHeight) :
				deviceContext(scene->getCLDeviceContext()), viewportWidth(vpWidth), viewportHeight(vpHeight) {
	setScene(scene);
	this->viewMatrix = cl::Buffer(deviceContext->context, CL_MEM_READ_ONLY, sizeof(cl_float)*16);

	resImg = cl::Image2D(deviceContext->context, CL_MEM_WRITE_ONLY, cl::ImageFormat(CL_RGBA, CL_FLOAT),
							viewportWidth, viewportHeight, 0);
}

template <cl_device_type DEVICE_TYPE>
void renderer<DEVICE_TYPE>::renderToTexture(GLuint tex, cl_float viewMat[16]) {
	deviceContext->commandQueue.enqueueWriteBuffer(this->viewMatrix, true, 0, sizeof(cl_float)*16, viewMat);

	raytrace(scene->getTriangleBuffer(),
			scene->getSphereBuffer(),
			scene->getPointLightBuffer(),
			scene->getMaterialBuffer(),
			viewMatrix, resImg);

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

	deviceContext->commandQueue.enqueueReadImage(
			resImg, true, origin, size, 0, 0, pixels);

	glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);
}

#endif /* RENDERER_H_ */
