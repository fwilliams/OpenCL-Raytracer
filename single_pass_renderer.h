/*
 * single_pass_renderer.h
 *
 *  Created on: Mar 18, 2014
 *      Author: francis
 */

#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <GL/gl.h>
#include <memory>
#include <vector>
#include <string>

#include "scene.h"

#ifndef SINGLE_PASS_RENDERER_H_
#define SINGLE_PASS_RENDERER_H_

template <cl_device_type DEVICE_TYPE>
struct SinglePassRenderer {
	SinglePassRenderer(std::shared_ptr<Scene<DEVICE_TYPE>> scene,
					  size_t vpWidth, size_t vpHeight,
					  unsigned reflectivePasses, double maxRenderDistance);


	void renderToTexture(GLuint tex, cl_float viewMat[16]);

	void setScene(std::shared_ptr<Scene<DEVICE_TYPE>> newScene) {
		scene = newScene;

		// Create the program
		std::map<std::string, std::string> defines {
			{"NUM_SPHERES", std::to_string(scene->getNumSpheres())},
			{"NUM_TRIANGLES", std::to_string(scene->getNumTriangles())},
			{"NUM_LIGHTS", std::to_string(scene->getNumPointLights())},
			{"NUM_MATERIALS", std::to_string(scene->getNumMaterials())},
			{"NUM_REFLECTIVE_PASSES", std::to_string(numReflectivePasses)},
			{"MAX_RENDER_DISTANCE", std::to_string(100.0)}
		};

		cl::Program program = scene->getCLDeviceContext()->createProgramFromFile(KERNEL_FILE_NAME, defines, OPENCL_BASE_DIR);
		kernelFunc = cl::KernelFunctor(cl::Kernel(program, "raytrace"), deviceContext->commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);
	}

private:
	const std::string KERNEL_FILE_NAME = "reflectracer.cl";
	const std::string OPENCL_BASE_DIR = "opencl";

	std::shared_ptr<Scene<CL_DEVICE_TYPE_GPU>> scene;
	std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> deviceContext;
	unsigned numReflectivePasses;
	double maxRenderDistance;

	size_t viewportWidth, viewportHeight;
	cl::KernelFunctor kernelFunc;
	cl::Buffer resImg;
};

template <cl_device_type DEVICE_TYPE>
SinglePassRenderer<DEVICE_TYPE>::SinglePassRenderer(std::shared_ptr<Scene<DEVICE_TYPE>> scene,
		size_t vpWidth, size_t vpHeight, unsigned numReflectivePasses, double maxRenderDistance) :
				deviceContext(scene->getCLDeviceContext()), numReflectivePasses(numReflectivePasses),
				maxRenderDistance(maxRenderDistance), viewportWidth(vpWidth), viewportHeight(vpHeight) {
	setScene(scene);
	this->resImg = cl::Buffer(deviceContext->context, CL_MEM_READ_WRITE, sizeof(cl_float4)*viewportWidth*viewportHeight);
}

template <cl_device_type DEVICE_TYPE>
void SinglePassRenderer<DEVICE_TYPE>::renderToTexture(GLuint tex, cl_float viewMat[16]) {
	kernelFunc(
			scene->getTriangleBuffer(),
			scene->getSphereBuffer(),
			scene->getPointLightBuffer(),
			scene->getMaterialBuffer(),
			resImg);


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



#endif /* SINGLE_PASS_RENDERER_H_ */
