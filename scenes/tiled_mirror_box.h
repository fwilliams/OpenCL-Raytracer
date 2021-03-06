/*
 * tiled_mirror_box.h
 *
 *  Created on: Mar 31, 2014
 *      Author: francis
 */

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <array>
#include <algorithm>

#include "data_types.h"
#include "box.h"

#ifndef TILED_MIRROR_BOX_H_
#define TILED_MIRROR_BOX_H_

namespace TiledMirrorBox {

template <BRDFType BRDF>
constexpr std::array<Material<BRDF>, 8>  DefaultMaterials(const std::array<TextureHdl, 6>& faceTexs) {
	return std::array<Material<BRDF>, 8>{{
		Material<BRDF> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, faceTexs[0]},
		Material<BRDF> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, faceTexs[1]},
		Material<BRDF> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, faceTexs[2]},
		Material<BRDF> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, faceTexs[3]},
		Material<BRDF> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, faceTexs[4]},
		Material<BRDF> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, faceTexs[5]},
		Material<BRDF> {
			cl_float3{{0.5, 0.5, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.2, 0.2, 0.6}},
			cl_float3{{0.5, 0.5, 0.5}},
			1000.0, no_texture},
		Material<BRDF> {
			cl_float3{{0.5, 0.5, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.6, 0.2, 0.2}},
			cl_float3{{0.5, 0.5, 0.5}},
			500.0, no_texture}
	}};
}

template <>
constexpr std::array<Material<DIFFUSE>, 8>  DefaultMaterials<DIFFUSE>(const std::array<TextureHdl, 6>& faceTexs) {
	return std::array<Material<DIFFUSE>, 8>{{
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.1}},
			faceTexs[0]},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.1}},
			faceTexs[1]},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.1, 0.5}},
			faceTexs[2]},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.5}},
			faceTexs[3]},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.5, 0.1}},
			faceTexs[4]},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.5}},
			faceTexs[5]},
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

#define TOGGLE(i) glm::pow( -1.0f, static_cast<float>( (glm::abs(i)%2)) )


void makeTranslationalFundamentalDomain(
		const glm::vec3& wallSize,
		std::vector<Triangle>& newTris,
		std::vector<Sphere>& newSpheres,
		std::vector<PointLight>& newLights,
		std::vector<Triangle>& tris,
		std::vector<Sphere>& spheres,
		std::vector<PointLight>& lights) {

	for(unsigned i = 0; i < tris.size(); i += 2) {
		glm::mat4 tx = glm::scale(glm::mat4(1.0), glm::vec3(-tris[i].normal.s[0], -tris[i].normal.s[1], -tris[i].normal.s[2]));

		std::transform(tris.begin(), tris.begin()+newTris.size(), newTris.begin(),
				[&tx](Triangle val){return transform<Triangle>(val, tx);});
		tris.insert(tris.end(), newTris.begin(), newTris.end());

		std::transform(spheres.begin(), spheres.begin()+newSpheres.size(), newSpheres.begin(),
				[&tx](Sphere val){return transform<Sphere>(val, tx);});
		spheres.insert(spheres.end(), newSpheres.begin(), newSpheres.end());

		std::transform(lights.begin(), lights.begin()+newLights.size(), newLights.begin(),
				[&tx](PointLight val){return transform<PointLight>(val, tx);});
		lights.insert(lights.end(), newLights.begin(), newLights.end());
	}
}

inline void makeTile(int i, int j,
		const glm::vec3& wallSize,
		std::vector<Triangle>& newTris,
		std::vector<Sphere>& newSpheres,
		std::vector<PointLight>& newLights,
		std::vector<Triangle>& tris,
		std::vector<Sphere>& spheres,
		std::vector<PointLight>& lights) {

	glm::mat4 tx =
			glm::translate(glm::mat4(1.0), glm::vec3(static_cast<float>(i)*wallSize.x, 0.0, static_cast<float>(j)*wallSize.z)) *
			glm::scale(glm::mat4(1.0), glm::vec3(TOGGLE(i), 1.0, TOGGLE(j)));

	std::transform(tris.begin(), tris.begin()+newTris.size(), newTris.begin(),
			[&tx](Triangle val){transform<Triangle>(val, tx); return val;});
	tris.insert(tris.end(), newTris.begin(), newTris.end());

	std::transform(spheres.begin(), spheres.begin()+newSpheres.size(), newSpheres.begin(),
			[&tx](Sphere val){transform<Sphere>(val, tx); return val;});
	spheres.insert(spheres.end(), newSpheres.begin(), newSpheres.end());

	std::transform(lights.begin(), lights.begin()+newLights.size(), newLights.begin(),
			[&tx](PointLight val){transform<PointLight>(val, tx); return val;});
	lights.insert(lights.end(), newLights.begin(), newLights.end());
}

template<BRDFType BRDF>
Scene<BRDF> buildTiledMirrorBox(
		Scene<BRDF>& scene,
		const glm::vec3& wallSize, const glm::ivec2& numTiles,
		const std::array<TextureHdl, 6>& faceTextures) {

	auto mats = DefaultMaterials<BRDF>(faceTextures);
	unsigned matOffset = scene.materials.size();
	scene.materials.insert(scene.materials.end(), &mats[0], &mats[0]+8);

	glm::vec3 halfSize = wallSize/2.0f;

	// Spheres
	std::vector<Sphere> spheres;
	float radius = std::min(std::min(wallSize.x, wallSize.y), wallSize.z) / 10.0f;

	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/8.0f, -halfSize.y+radius, -wallSize.z/8.0f}}, 6});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/8.0f, -halfSize.y+radius, -wallSize.z/8.0f}}, 7});
	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/8.0f, -halfSize.y+radius,  wallSize.z/8.0f}}, 7});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/8.0f, -halfSize.y+radius,  wallSize.z/8.0f}}, 6});

	// Lights
	std::vector<PointLight> lights;
	glm::vec3 d = wallSize / 30.0f;
	lights.push_back(
			PointLight{
				cl_float3{{wallSize.x/4.0f, halfSize.y - d.y, wallSize.z/4.0f}},
				cl_float3{{0.75, 0.25, 0.25}},
				1.0f/halfSize.y});
	lights.push_back(
			PointLight{
				cl_float3{{-wallSize.x/4.0f, halfSize.y - d.y, wallSize.z/4.0f}},
				cl_float3{{0.25, 0.75, 0.25}},
				1.0f/halfSize.y});
	lights.push_back(
			PointLight{
				cl_float3{{wallSize.x/4.0f, halfSize.y - d.y, -wallSize.z/4.0f}},
				cl_float3{{0.25, 0.25, 0.75}},
				1.0f/halfSize.y});
	lights.push_back(
			PointLight{
				cl_float3{{-wallSize.x/4.0f, halfSize.y - d.y, -wallSize.z/4.0f}},
				cl_float3{{0.75, 0.25, 0.75}},
				1.0f/halfSize.y});

	// Box
	std::vector<Triangle> tris;
	if(numTiles == glm::ivec2(0, 0)) {
		Box::makeBoxInterior(tris, wallSize, matOffset);
	} else if(numTiles == glm::ivec2(0, 1)) {
		Box::makePartialBoxInterior<1>(
				tris, wallSize, {{Box::Face::FRONT}},
				{{matOffset, matOffset+1, matOffset+3, matOffset+4, matOffset+5}});
	} else if(numTiles == glm::ivec2(1, 0)) {
		Box::makePartialBoxInterior<1>(
				tris, wallSize, {{Box::Face::RIGHT}},
				{{matOffset, matOffset+1, matOffset+2, matOffset+3, matOffset+5}});
	} else if(numTiles.x == 0) {
		Box::makePartialBoxInterior<2>(
				tris, wallSize, {{Box::Face::FRONT, Box::Face::BACK}},
				{{matOffset+0, matOffset+1, matOffset+4, matOffset+5}});
	} else if(numTiles.y == 0) {
		Box::makePartialBoxInterior<2>(
				tris, wallSize, {{Box::Face::RIGHT, Box::Face::LEFT}}, {{matOffset, matOffset+1}});
	} else {
		Box::makePartialBoxInterior<4>(
				tris, wallSize,
				{{Box::Face::FRONT, Box::Face::BACK, Box::Face::RIGHT, Box::Face::LEFT}});
	}

	std::vector<Triangle> newTris(tris.size());
	std::vector<Sphere> newSpheres(spheres.size());
	std::vector<PointLight> newLights(lights.size());

	for(int i = -numTiles.x; i <= numTiles.x; i++) {
		for(int j = -numTiles.y; j <= numTiles.y; j++) {
			if(i != 0 || j != 0) {
				makeTile(i, j, wallSize,
						 newTris, newSpheres, newLights,
						 tris, spheres, lights);

			}
		}
	}

	scene.geometry.addTriangles(tris.begin(), tris.end());
	scene.geometry.addSpheres(spheres.begin(), spheres.end());
	scene.lights.insert(scene.lights.end(), lights.begin(), lights.end());
	return scene;
}

}

#endif /* TILED_MIRROR_BOX_H_ */
