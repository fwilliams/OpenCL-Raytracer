/*
 * multi_pass_renderer.h
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
#include <gli/gli.hpp>

#include <iostream>

#include "scene.h"
#include "texture_atlas.h"

#ifndef MULTI_PASS_RENDERER_H_
#define MULTI_PASS_RENDERER_H_

template <cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
struct MultiPassRenderer {
	MultiPassRenderer(std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> scene,
					  size_t vpWidth, size_t vpHeight,
					  unsigned reflectivePasses, double maxRenderDistance);


	void renderToTexture(GLuint tex, cl_float16 viewMatrix);

	void setScene(std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> newScene) {
		scene = newScene;

		// Create the program
		std::map<std::string, std::string> defines {
			{"NUM_SPHERES", std::to_string(scene->getNumSpheres())},
			{"NUM_TRIANGLES", std::to_string(scene->getNumTriangles())},
			{"NUM_LIGHTS", std::to_string(scene->getNumPointLights())},
			{"NUM_MATERIALS", std::to_string(scene->getNumMaterials())},
			{"MAX_RENDER_DISTANCE", std::to_string(maxRenderDistance)},
			{std::string(Material<LIGHT_MODEL>::name()), std::string()}
		};

		cl::Program firstPassProg = scene->getCLDeviceContext()->createProgramFromFile(FIRST_PASS_FILE_NAME, defines, OPENCL_BASE_DIR);
		firstPass = cl::KernelFunctor(cl::Kernel(firstPassProg, "first_pass"), deviceContext->commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);

		cl::Program reflectPassProg = scene->getCLDeviceContext()->createProgramFromFile(RFLCT_PASS_FILE_NAME, defines, OPENCL_BASE_DIR);
		reflectPass = cl::KernelFunctor(cl::Kernel(reflectPassProg, "reflect_pass"), deviceContext->commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);
	}

private:
	const std::string FIRST_PASS_FILE_NAME = "first_pass.cl";
	const std::string RFLCT_PASS_FILE_NAME = "reflect_pass.cl";
	const std::string OPENCL_BASE_DIR = "opencl";

	std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> scene;
	std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> deviceContext;
	unsigned numReflectivePasses;
	double maxRenderDistance;

	size_t viewportWidth, viewportHeight;
	cl::KernelFunctor firstPass, reflectPass;
	cl::Buffer params, rayBuffer, reflectivityBuffer;
	cl::Buffer resImg;
	cl::Image2D textureAtlas;
};

template <cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
MultiPassRenderer<DEVICE_TYPE, LIGHT_MODEL>::MultiPassRenderer(std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> scene,
		size_t vpWidth, size_t vpHeight, unsigned numReflectivePasses, double maxRenderDistance) :
				deviceContext(scene->getCLDeviceContext()), numReflectivePasses(numReflectivePasses),
				maxRenderDistance(maxRenderDistance), viewportWidth(vpWidth), viewportHeight(vpHeight) {
	setScene(scene);
	this->rayBuffer = cl::Buffer(deviceContext->context, CL_MEM_READ_WRITE, sizeof(Ray)*viewportWidth*viewportHeight);
	this->reflectivityBuffer = cl::Buffer(deviceContext->context, CL_MEM_READ_WRITE, sizeof(cl_float3)*viewportWidth*viewportHeight);
	this->resImg = cl::Buffer(deviceContext->context, CL_MEM_READ_WRITE, sizeof(cl_float4)*viewportWidth*viewportHeight);

	cl_float4 buf[512*512];
	for(unsigned i = 0; i < 512; i++) {
		for(unsigned j = 0; j < 512; j++) {
			buf[i*512+j] = cl_float4{{0.5f+static_cast<float>(i)/1024.0f,0.5f+static_cast<float>(j)/1024.0f, 0.5f, 1.0f}};
		}
	}


	TextureAtlas texAtlas;
	texAtlas.createTexture(gli::load_dds("textures/tex1.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex2.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex3.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex1.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex2.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex3.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex1.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex2.dds"));
	texAtlas.createTexture(gli::load_dds("textures/tex3.dds"));
	texAtlas.createTexture(gli::load_dds("textures/swasticock.dds"));

	textureAtlas = texAtlas.packAtlas(*deviceContext);
}

template <cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
void MultiPassRenderer<DEVICE_TYPE, LIGHT_MODEL>::renderToTexture(GLuint tex, cl_float16 viewMatrix) {
	firstPass(rayBuffer,
			reflectivityBuffer,
			scene->getTriangleBuffer(),
			scene->getSphereBuffer(),
			scene->getPointLightBuffer(),
			scene->getMaterialBuffer(),
			textureAtlas,
			resImg,
			viewMatrix);

	for(int i = 0; i < numReflectivePasses; ++i) {
		reflectPass(rayBuffer,
				reflectivityBuffer,
				scene->getTriangleBuffer(),
				scene->getSphereBuffer(),
				scene->getPointLightBuffer(),
				scene->getMaterialBuffer(),
				textureAtlas,
				resImg);
	}

	glBindTexture(GL_TEXTURE_2D, tex);

	cl_float4 pixels[viewportWidth*viewportHeight];

	deviceContext->commandQueue.enqueueReadBuffer(
		resImg, true, 0, viewportWidth*viewportHeight*sizeof(cl_float4), pixels);

	glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);
}

#endif /* MULTI_PASS_RENDERER_H_ */
