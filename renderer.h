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

struct renderer {
	renderer(std::shared_ptr<Scene<std::vector>> scene,
			RenderParams& params, unsigned vpWidth,
			unsigned vpHeight);

	cl::KernelFunctor raytrace;

	GLuint getGLTexture() {
		return glResTexture;
	}

	void renderToTexture(GLuint tex, cl_float viewMat[16]);

	void resizeViewport(unsigned vpWidth, unsigned vpHeight);

private:
	std::shared_ptr<Scene<std::vector>> scene;
	size_t viewportWidth, viewportHeight;
	cl::Context ctx;
	cl::Device device;
	cl::CommandQueue cmdQueue;
	cl::Kernel kernel;
	cl::Program program;
	cl::Buffer params, viewMatrix;
	cl::Image2D resImg;
	cl_uint numSpheres, numTris, numLights;
	GLuint glResTexture;

	cl::Program createProgramFromFile(std::string& filename);

	void initOpenCL();

	void packBuffers(RenderParams& renderparams);
};

#endif /* RENDERER_H_ */
