/*
 * kaleidescope.h
 *
 *  Created on: Apr 10, 2014
 *      Author: francis
 */

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <algorithm>

#include "renderer/data_types.h"

#ifndef KALEIDESCOPE_H_
#define KALEIDESCOPE_H_

class Kaleidescope {
public:

	Kaleidescope(const std::vector<unsigned>& kpoints, float scale) {
	}

	std::vector<Triangle> triangles;
	// Need ceiling and floor material
	// Need way of putting lights and spheres in scene

	std::vector<Triangle> triangleFromAngles(const std::array<unsigned, 3>& kpoints, float scale, float depth) {
		auto minAngleIter = std::max_element(kpoints.begin(), kpoints.end());
		auto nextAngleIter = minAngleIter;

		float minAngle = glm::pi<float>() / static_cast<float>(*minAngleIter);
		std::vector<Triangle> tris;
		Triangle t;

		glm::mat4 rotate = glm::rotate(glm::mat4(1.0), minAngle, glm::vec3(0.0, 0.0, 1.0));
		glm::mat4 rotate2 = glm::rotate(glm::mat4(1.0), 2.0f * minAngle, glm::vec3(0.0, 0.0, 1.0));
		glm::mat4 invRotate = glm::rotate(glm::mat4(1.0), -minAngle, glm::vec3(0.0, 0.0, 1.0));

		float sinA = glm::sin(minAngle);

		t.v1 = cl_float3{{0.0f, 0.0f, depth}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		std::cout << *nextAngleIter << std::endl;
		glm::vec2 v2 = glm::vec2(scale*glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA, 0.0);
		t.v2 = cl_float3{{v2.x, v2.y, depth}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		std::cout << *nextAngleIter << std::endl;
		glm::vec4 direction = rotate * glm::vec4(1.0, 0.0, 0.0, 0.0);
		std::cout << glm::to_string(direction) << std::endl;
		glm::vec2 v3 = glm::vec2(direction) * scale * (glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA);
		t.v3 = cl_float3{{v3.x, v3.y, depth}};

		glm::vec3 n(0.0, 0.0, 1.0);
		t.normal = cl_float3{{n.x, n.y, n.z}};
		t.materialId = 0;

		tris.push_back(t);


		glm::mat4 tx = rotate * glm::scale(glm::mat4(1.0), glm::vec3(1.0, -1.0, 1.0)) * invRotate;

		Triangle t2 = t;
		t2.materialId = 1;
		transform<Triangle>(t2, tx);
		tris.push_back(t2);

		for(unsigned i = 0; i < *minAngleIter; i++) {
			Triangle t3= tris[tris.size()-2];
			Triangle t4 = tris[tris.size()-1];
			t3.materialId = 0;
			t4.materialId = 1;

			transform<Triangle>(t3, rotate2);
			transform<Triangle>(t4, rotate2);

			tris.push_back(t3);
			tris.push_back(t4);
		}


		return tris;
	}
};



#endif /* KALEIDESCOPE_H_ */
