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

struct Sphere {
	double radius;
	glm::vec3 position;
};

struct Triangle {
	glm::vec3 vertices[3];
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
			const std::vector<PointLight>& lights);

	void renderToTexture();

private:
	cl::Context ctx;
	cl::CommandQueue cmdQueue;
	cl::Program program;
	cl::Buffer geometry, lights;
	glm::mat4 viewTransform;
	cl_uint numSpheres, numTris, numLights;

	inline void packBuffers(const std::vector<Sphere>& spheres,
			const std::vector<Triangle>& tris,
			const std::vector<PointLight>& lights);
};

#endif /* RENDERER_H_ */
