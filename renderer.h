/*
 * renderer.h
 *
 *  Created on: Feb 24, 2014
 *      Author: francis
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

struct Sphere {
	double radius;
	glm::vec3 position;
	unsigned materialId;
};

struct Triangle {
	glm::vec3 vertices[3];
	unsigned materialId;
};

struct PointLight {
	glm::vec3 position;
	glm::vec3 power;
};

struct Material {
	glm::vec3 reflection;
	glm::vec3 refraction;
	glm::vec3 absorbtion;
};

struct renderer {
	renderer(const std::vector<Sphere>& spheres,
			const std::vector<Triangle>& tris,
			const std::vector<PointLight>& lights,
			const std::vector<Material>& materials);

	cl::KernelFunctor raytrace;

	void renderToTexture();

private:
	size_t viewportWidth, viewportHeight;
	cl::Context ctx;
	cl::CommandQueue cmdQueue;
	cl::Program program;
	cl::Buffer tris, spheres, lights, materials;
	cl::Kernel kernel;
	cl::Device device;
	glm::mat4 viewTransform;
	cl_uint numSpheres, numTris, numLights;

	cl::Program createProgramFromFile(std::string& filename);

	void initOpenCL();

	void packBuffers(const std::vector<Sphere>& spheres,
			const std::vector<Triangle>& tris,
			const std::vector<PointLight>& lights,
			const std::vector<Material>& materials);
};

#endif /* RENDERER_H_ */
