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
	constexpr std::array<Material<BRDF>, 2> defaultMaterials(TextureHdl tex_id) {
		return std::array<Material<BRDF>, 2>{{
			Material<BRDF> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{1.0, 1.0, 1.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				1.0, tex_id},
			Material<BRDF> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.2, 0.2, 0.6}},
				cl_float3{{0.5, 0.5, 0.5}},
				1000.0, no_texture}
		}};
	}

	template <>
	constexpr std::array<Material<DIFFUSE>, 2>  defaultMaterials<DIFFUSE>(TextureHdl tex_id) {
		return std::array<Material<DIFFUSE>, 2>{{
			Material<DIFFUSE> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.6, 0.6, 0.6}},
				tex_id},
			Material<DIFFUSE> {
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.2, 0.2, 0.6}},
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

	void triangleFromAngles(const std::array<unsigned, 3>& kpoints, float scale, float depth,
			std::vector<Quad>& quads, std::vector<Triangle>& triangles, std::vector<PointLight>& lights, std::vector<Sphere>& spheres) {
		auto minAngleIter = std::max_element(kpoints.begin(), kpoints.end());
		auto nextAngleIter = minAngleIter;

		float minAngle = glm::pi<float>() / static_cast<float>(*minAngleIter);
		Triangle t;

		glm::mat4 rotateHalf = glm::rotate(glm::mat4(1.0), minAngle/2.0f, glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0), minAngle, glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 rotate2 = glm::rotate(glm::mat4(1.0), 2.0f * minAngle, glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 invRotate = glm::rotate(glm::mat4(1.0), -minAngle, glm::vec3(0.0, 1.0, 0.0));

		float sinA = glm::sin(minAngle);

		t.vp1 = cl_float3{{0.0f, depth, 0.0f}};
		t.vt1 = cl_float2{{0.0f, 0.0f}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		float lenB = scale * glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA;
		t.vp2 = cl_float3{{lenB, depth, 0.0}};
		t.vt1 = cl_float2{{1.0f, 0.0f}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		float lenC = scale * (glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA);
		glm::vec4 v3 = lenC * rotate * glm::vec4(1.0, 0.0, 0.0, 0.0);
		t.vp3 = cl_float3{{v3.x, depth, v3.z}};
		t.vt3 = cl_float2{{1.0, 1.0}};

		glm::vec3 n(0.0, -1.0, 0.0);
		t.normal = cl_float3{{n.x, n.y, n.z}};
		triangles.push_back(t);

		glm::vec3 quadNormal = glm::mat3(rotateHalf) * glm::vec3(1.0, 0.0, 0.0);
		quads.push_back(
				Quad{
					t.vp2, t.vp3,
					cl_float3{{t.vp3.s[0], -depth, t.vp3.s[2]}},
					cl_float3{{t.vp2.s[0], -depth, t.vp2.s[2]}},
					cl_float2{{0.0, 0.0}},  cl_float2{{1.0, 0.0}},
					cl_float2{{1.0, 1.0}},  cl_float2{{0.0, 1.0}},
					cl_float3{{-quadNormal.x, -quadNormal.y, -quadNormal.z}}, 0
				});

		glm::vec2 centroid((t.vp1.s[0] + t.vp2.s[0] + t.vp3.s[0])/3.0f, (t.vp1.s[2] + t.vp2.s[2] + t.vp3.s[2])/3.0f);
		spheres.push_back(Sphere{1.5f, cl_float3{{centroid.x, 0.0f, centroid.y}}, 1});
		lights.push_back(PointLight{cl_float3{{centroid.x, depth-0.5f, centroid.y}}, cl_float3{{0.3f, 0.3f, 0.3f}}, 0.5});

		glm::mat4 tx = rotate * glm::scale(glm::mat4(1.0), glm::vec3(1.0, 1.0, -1.0)) * invRotate;
		triangles.push_back(transform<Triangle>(t, tx));
		spheres.push_back(transform<Sphere>(spheres.back(), tx));
		lights.push_back(transform<PointLight>(lights.back(), tx));
		quads.push_back(transform<Quad>(quads.back(), tx));

		std::vector<Triangle> roof;
		for(unsigned i = 0; i < *minAngleIter; i++) {
			Triangle t3 = transform<Triangle>(triangles[triangles.size()-2], rotate2);
			Triangle t4 = transform<Triangle>(triangles[triangles.size()-1], rotate2);

			Triangle t5 = transform<Triangle>(t3, glm::scale(glm::mat4(1.0), glm::vec3(1.0, -1.0, 1.0f)));
			Triangle t6 = transform<Triangle>(t4, glm::scale(glm::mat4(1.0), glm::vec3(1.0, -1.0, 1.0f)));

			PointLight l3 = transform<PointLight>(lights[lights.size()-2], rotate2);
			PointLight l4 = transform<PointLight>(lights[lights.size()-1], rotate2);

			Sphere s3 = transform<Sphere>(spheres[spheres.size()-2], rotate2);
			Sphere s4 = transform<Sphere>(spheres[spheres.size()-1], rotate2);

			Quad q3 = transform<Quad>(quads[quads.size()-2], rotate2);
			Quad q4 = transform<Quad>(quads[quads.size()-1], rotate2);

			triangles.push_back(t3);
			triangles.push_back(t4);

			quads.push_back(q3);
			quads.push_back(q4);

			lights.push_back(l3);
			lights.push_back(l4);

			spheres.push_back(s3);
			spheres.push_back(s4);

			roof.push_back(t5);
			roof.push_back(t6);
		}

		triangles.insert(triangles.end(), roof.begin(), roof.end());
	}

	template <BRDFType BRDF>
	void buildKaleidescope(Scene<BRDF>& scene, const std::array<unsigned, 3>& kpoints, float scale, float depth, TextureHdl tex = no_texture) {
		auto mats = defaultMaterials<BRDF>(tex);
		std::vector<Triangle> tris;
		std::vector<PointLight> lights;
		std::vector<Sphere> spheres;
		std::vector<Quad> quads;
		triangleFromAngles(kpoints, scale, depth, quads, tris, lights, spheres);

		scene.materials.insert(scene.materials.end(), &mats[0], &mats[0]+mats.size());
		scene.geometry.addTriangles(tris.begin(), tris.end());
		scene.geometry.addSpheres(spheres.begin(), spheres.end());
//		scene.geometry.addQuads(quads.begin(), quads.end());
		scene.lights.insert(scene.lights.end(), lights.begin(), lights.end());
	}
};



#endif /* KALEIDESCOPE_H_ */
