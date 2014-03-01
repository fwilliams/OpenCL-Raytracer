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
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

struct Sphere {
	cl_float radius;
	cl_float3 position;
	cl_uint materialId;
};

struct Triangle {
	cl_float3 v1, v2, v3;
	cl_uint materialId;
};

struct PointLight {
	cl_float3 position;
	cl_float3 power;
};

struct Material {
	cl_float3 reflection;
	cl_float3 refraction;
	cl_float3 diffuseColor;;
};

struct RenderParams {
	cl_float maxRenderDist;
	cl_uint numTris;
	cl_uint numSpheres;
	cl_uint numLights;
};

struct renderer {
	renderer(const std::vector<Sphere>& spheres,
			const std::vector<Triangle>& tris,
			const std::vector<PointLight>& lights,
			const std::vector<Material>& materials,
			RenderParams& params, unsigned vpWidth,
			unsigned vpHeight);

	cl::KernelFunctor raytrace;

	GLuint getGLTexture() {
		return glResTexture;
	}

	void renderToTexture(GLuint tex, cl_float viewMatrix[16]);

	void resizeViewport(unsigned vpWidth, unsigned vpHeight);

private:
	size_t viewportWidth, viewportHeight;
	cl::Context ctx;
	cl::Device device;
	cl::CommandQueue cmdQueue;
	cl::Kernel kernel;
	cl::Program program;
	cl::Buffer tris, spheres, lights, materials, params, viewMatrix;
	cl::Image2D resImg;
	glm::mat4 viewTransform;
	cl_uint numSpheres, numTris, numLights;
	GLuint glResTexture;

	cl::Program createProgramFromFile(std::string& filename);

	void initOpenCL();

	void packBuffers(const std::vector<Sphere>& spheres,
			const std::vector<Triangle>& tris,
			const std::vector<PointLight>& lights,
			const std::vector<Material>& materials,
			RenderParams& renderparams);
};

#endif /* RENDERER_H_ */
