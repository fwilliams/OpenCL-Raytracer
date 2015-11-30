/*
 * multi_pass_renderer.h
 *
 *  Created on: Mar 17, 2014
 *      Author: francis
 */

#include <GL/gl.h>

#include <memory>
#include <vector>
#include <string>

#include "scene.h"
#include "detail/texture_atlas.h"
#include "detail/cl_device_context.h"

#ifndef RENDERER_H_
#define RENDERER_H_

template <cl_device_type DEVICE_TYPE, BRDFType BRDF>
struct Renderer {
	~Renderer() {
		std::cout << "Nuking renderer" << std::endl;
	}
	Renderer(Scene<BRDF>& scene,
			 size_t vpWidth, size_t vpHeight,
			 unsigned reflectivePasses, double maxRenderDistance) :
					deviceContext(ClDeviceContext<DEVICE_TYPE>()), numReflectivePasses(reflectivePasses),
					maxRenderDistance(maxRenderDistance), viewportWidth(vpWidth), viewportHeight(vpHeight) {

		// Create the program
		std::map<std::string, std::string> defines {
			{"NUM_SPHERES", std::to_string(scene.geometry.numSpheres())},
			{"NUM_TRIANGLES", std::to_string(scene.geometry.numTriangles())},
			{"NUM_LIGHTS", std::to_string(scene.lights.size())},
			{"NUM_MATERIALS", std::to_string(scene.materials.size())},
			{"MAX_RENDER_DISTANCE", std::to_string(maxRenderDistance)},
			{std::string(Material<BRDF>::name()), std::string()}
		};

		cl::Program firstPassProg = deviceContext.createProgramFromFile(FIRST_PASS_FILE_NAME, defines, OPENCL_BASE_DIR);
		firstPass = cl::KernelFunctor(cl::Kernel(firstPassProg, "first_pass"), deviceContext.commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);

		cl::Program reflectPassProg = deviceContext.createProgramFromFile(RFLCT_PASS_FILE_NAME, defines, OPENCL_BASE_DIR);
		reflectPass = cl::KernelFunctor(cl::Kernel(reflectPassProg, "reflect_pass"), deviceContext.commandQueue,
				cl::NullRange, cl::NDRange(viewportWidth, viewportHeight),
				cl::NullRange);

		texAtlas = TextureAtlas::makeTextureAtlas(deviceContext, scene.textures);
		packCLBuffers(scene);
	}

	void renderToTexture(GLuint tex, cl_float16 viewMatrix) {
		firstPass(rayBuf,
				reflectivityBuf,
				triBuf,
				sphereBuf,
				lightBuf,
				materialBuf,
				texOffsetBuf,
				texAtlas.data,
				resImg,
				viewMatrix);

		for(int i = 0; i < numReflectivePasses; ++i) {
			reflectPass(rayBuf,
					reflectivityBuf,
					triBuf,
					sphereBuf,
					lightBuf,
					materialBuf,
					texOffsetBuf,
					texAtlas.data,
					resImg);
		}


		cl_float4 pixels[viewportWidth*viewportHeight];

		deviceContext.commandQueue.enqueueReadBuffer(
			resImg, true, 0, viewportWidth*viewportHeight*sizeof(cl_float4), pixels);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(
				GL_TEXTURE_2D,
				0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_FLOAT, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);

		deviceContext.commandQueue.finish();
	}

private:
	const std::string FIRST_PASS_FILE_NAME = "first_pass.cl";
	const std::string RFLCT_PASS_FILE_NAME = "reflect_pass.cl";
	const std::string OPENCL_BASE_DIR = "renderer/include/detail/opencl";

	ClDeviceContext<DEVICE_TYPE> deviceContext;
	unsigned numReflectivePasses;
	double maxRenderDistance;

	size_t viewportWidth, viewportHeight;
	cl::KernelFunctor firstPass, reflectPass;
	cl::Buffer rayBuf, reflectivityBuf, resImg, triBuf, sphereBuf, lightBuf, materialBuf, texOffsetBuf;
	TextureAtlas texAtlas;

	inline void packCLBuffers(Scene<BRDF>& scene) {
		rayBuf = cl::Buffer(deviceContext.context, CL_MEM_READ_WRITE, sizeof(Ray)*viewportWidth*viewportHeight);
		reflectivityBuf = cl::Buffer(deviceContext.context, CL_MEM_READ_WRITE, sizeof(cl_float3)*viewportWidth*viewportHeight);
		resImg = cl::Buffer(deviceContext.context, CL_MEM_READ_WRITE, sizeof(cl_float4)*viewportWidth*viewportHeight);

		if(scene.geometry.numSpheres() != 0) {
			size_t sphereByteSize = sizeof(Sphere) * scene.geometry.numSpheres();
			sphereBuf = cl::Buffer(deviceContext.context, CL_MEM_READ_ONLY, sphereByteSize);
			deviceContext.commandQueue.enqueueWriteBuffer(
					sphereBuf, true, 0, sphereByteSize, scene.geometry.sphereData());
		}

		if(scene.geometry.numTriangles() != 0) {
			size_t trisByteSize = sizeof(Triangle) * scene.geometry.numTriangles();
			triBuf = cl::Buffer(deviceContext.context, CL_MEM_READ_ONLY, trisByteSize);
			deviceContext.commandQueue.enqueueWriteBuffer(
					triBuf, true, 0, trisByteSize, scene.geometry.triData());
		}

		if(!scene.lights.empty()) {
			size_t lightByteSize = sizeof(PointLight) * scene.lights.size();
			lightBuf = cl::Buffer(deviceContext.context, CL_MEM_READ_ONLY, lightByteSize);
			deviceContext.commandQueue.enqueueWriteBuffer(
					lightBuf, true, 0, lightByteSize, scene.lights.data());
		}

		if(!scene.materials.empty()) {
			size_t materialByteSize = sizeof(Material<BRDF>) * scene.materials.size();
			materialBuf = cl::Buffer(deviceContext.context, CL_MEM_READ_ONLY, materialByteSize);
			deviceContext.commandQueue.enqueueWriteBuffer(
					materialBuf, true, 0, materialByteSize, scene.materials.data());
		}

		if(!texAtlas.texPositions.empty()) {
			std::vector<cl_float4> textureOffsets;
			std::for_each(texAtlas.texPositions.begin(), texAtlas.texPositions.end(),
					[&](glm::ivec4& rect) {
						cl_float4 frect = cl_float4{
							{static_cast<float>(rect.x), static_cast<float>(rect.y),
							 static_cast<float>(rect.z), static_cast<float>(rect.w)}};
					textureOffsets.push_back(frect);
			});

			size_t texOffsetByteSize = sizeof(cl_float4) * textureOffsets.size();
			texOffsetBuf = cl::Buffer(deviceContext.context, CL_MEM_READ_ONLY, texOffsetByteSize);
			deviceContext.commandQueue.enqueueWriteBuffer(
					texOffsetBuf, true, 0, texOffsetByteSize, textureOffsets.data());
		}
	}
};

#endif /* RENDERER_H_ */
