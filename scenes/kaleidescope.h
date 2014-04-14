/*
 * kaleidescope.h
 *
 *  Created on: Apr 10, 2014
 *      Author: francis
 */

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <algorithm>

#include "renderer.h"

#ifndef KALEIDESCOPE_H_
#define KALEIDESCOPE_H_

namespace Kaleidescope {
	template <BRDFType BRDF>
	constexpr std::array<Material<BRDF>, 2> defaultMaterials() {
		return std::array<Material<BRDF>, 2>{{
			Material<BRDF> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.2, 0.2, 0.6}},
				cl_float3{{0.0, 0.0, 0.0}},
				1000.0, no_texture},
			Material<BRDF> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.6, 0.2, 0.2}},
				cl_float3{{0.0, 0.0, 0.0}},
				500.0, no_texture}
		}};
	}

	template <>
	constexpr std::array<Material<DIFFUSE>, 2>  defaultMaterials<DIFFUSE>() {
		return std::array<Material<DIFFUSE>, 2>{{
			Material<DIFFUSE> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.2, 0.2, 0.6}},
				no_texture},
			Material<DIFFUSE> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.6, 0.2, 0.2}},
				no_texture}
			}};
	}

	template <typename RandomAccessIterator>
	RandomAccessIterator cyclic_next(RandomAccessIterator it, RandomAccessIterator cycleBegin, RandomAccessIterator cycleEnd){
		if(it == cycleEnd - 1) {
			return cycleBegin;
		} else {
			return it + 1;
		}
	}

	void triangleFromAngles(const std::array<unsigned, 3>& kpoints, float scale, float depth, std::vector<Triangle>& triangles) {
		auto minAngleIter = std::max_element(kpoints.begin(), kpoints.end());
		auto nextAngleIter = minAngleIter;

		float minAngle = glm::pi<float>() / static_cast<float>(*minAngleIter);
		Triangle t;

		glm::mat4 rotate = glm::rotate(glm::mat4(1.0), minAngle, glm::vec3(0.0, 0.0, 1.0));
		glm::mat4 rotate2 = glm::rotate(glm::mat4(1.0), 2.0f * minAngle, glm::vec3(0.0, 0.0, 1.0));
		glm::mat4 invRotate = glm::rotate(glm::mat4(1.0), -minAngle, glm::vec3(0.0, 0.0, 1.0));

		float sinA = glm::sin(minAngle);

		t.vp1 = cl_float3{{0.0f, 0.0f, depth}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		glm::vec2 v2 = glm::vec2(scale*glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA, 0.0);
		t.vp2 = cl_float3{{v2.x, v2.y, depth}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		glm::vec4 direction = rotate * glm::vec4(1.0, 0.0, 0.0, 0.0);
		glm::vec2 v3 = glm::vec2(direction) * scale * (glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA);
		t.vp3 = cl_float3{{v3.x, v3.y, depth}};

		glm::vec3 n(0.0, 0.0, 1.0);
		t.normal = cl_float3{{n.x, n.y, n.z}};
		t.materialId = 0;

		triangles.push_back(t);

		glm::mat4 tx = rotate * glm::scale(glm::mat4(1.0), glm::vec3(1.0, -1.0, 1.0)) * invRotate;
		Triangle t2 = transform<Triangle>(t, tx);
		t2.materialId = 1;
		triangles.push_back(t2);

		for(unsigned i = 0; i < *minAngleIter; i++) {
			Triangle t3 = transform<Triangle>(triangles[triangles.size()-2], rotate2);
			Triangle t4 =transform<Triangle>(triangles[triangles.size()-1], rotate2);

			t3.materialId = 0;
			t4.materialId = 1;

			triangles.push_back(t3);
			triangles.push_back(t4);
		}
	}

	template <BRDFType BRDF>
	Scene<BRDF> buildKaleidescope(const std::array<unsigned, 3>& kpoints, float scale, float depth) {
		auto mats = defaultMaterials<BRDF>();
		std::vector<Triangle> tris;
		triangleFromAngles(kpoints, scale, depth, tris);

		std::vector<PointLight> lights;
		lights.push_back(PointLight{ cl_float3{{0.0f, 0.0f, 5.0f}}, cl_float3{{0.7f, 0.7f, 0.7f}}, 0.01f });

		Scene<BRDF> scene {
			tris, std::vector<Sphere>(), lights
		};
		scene.materials = std::vector<Material<BRDF>>(&mats[0], &mats[0]+mats.size());

		return scene;
	}
};



#endif /* KALEIDESCOPE_H_ */
