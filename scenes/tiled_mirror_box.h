/*
 * tiled_mirror_box.h
 *
 *  Created on: Mar 31, 2014
 *      Author: francis
 */

#include <array>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "scene.h"
#include "box.h"

#include <iostream>

#ifndef TILED_MIRROR_BOX_H_
#define TILED_MIRROR_BOX_H_

namespace TiledMirrorBox {

template <LightModel LIGHT_MODEL>
constexpr std::array<Material<LIGHT_MODEL>, 8>  DefaultMaterials() {
	return std::array<Material<LIGHT_MODEL>, 8>{{
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, 1},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, 1},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, 1},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, 1},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, 1},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{1.0, 1.0, 1.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0, 1},
		Material<LIGHT_MODEL> {
			cl_float3{{0.5, 0.5, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.2, 0.2, 0.6}},
			cl_float3{{0.5, 0.5, 0.5}},
			1000.0, 0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.5, 0.5, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.6, 0.2, 0.2}},
			cl_float3{{0.5, 0.5, 0.5}},
			500.0, 0}
	}};
}

template <>
constexpr std::array<Material<DIFFUSE>, 8>  DefaultMaterials<DIFFUSE>() {
	return std::array<Material<DIFFUSE>, 8>{{
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.1}},
			0},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.1}},
			0},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.1, 0.5}},
			0},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.5}},
			0},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.5, 0.1}},
			0},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.5}},
			0},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.2, 0.2, 0.6}},
			0},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.6, 0.2, 0.2}},
			0}
		}};
}


#define TOGGLE(i) glm::pow( -1.0f, static_cast<float>( (glm::abs(i)%2)) )

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

template<cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> buildTiledMirrorBox(
		const glm::vec3& wallSize, const glm::ivec2& numTiles,
		const std::array<Material<LIGHT_MODEL>, 8>& materials = DefaultMaterials<LIGHT_MODEL>()) {

	std::vector<Triangle> tris;
	std::vector<Sphere> spheres;
	std::vector<PointLight> lights;

	glm::vec3 halfSize = wallSize/2.0f;

	// Spheres
	float radius = std::min(std::min(wallSize.x, wallSize.y), wallSize.z) / 10.0f;

	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/8.0f, -halfSize.y+radius, -wallSize.z/8.0f}}, 6});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/8.0f, -halfSize.y+radius, -wallSize.z/8.0f}}, 7});
	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/8.0f, -halfSize.y+radius,  wallSize.z/8.0f}}, 7});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/8.0f, -halfSize.y+radius,  wallSize.z/8.0f}}, 6});

	// Lights
	glm::vec3 d = wallSize / 30.0f;
	lights.push_back(
			PointLight{
				cl_float3{{wallSize.x/4.0f, halfSize.y - d.y, wallSize.z/4.0f}},
				cl_float3{{0.5, 0.5, 0.5}},
				1.0f/halfSize.y});
	lights.push_back(
			PointLight{
				cl_float3{{-wallSize.x/4.0f, halfSize.y - d.y, wallSize.z/4.0f}},
				cl_float3{{0.5, 0.5, 0.5}},
				1.0f/halfSize.y});
	lights.push_back(
			PointLight{
				cl_float3{{wallSize.x/4.0f, halfSize.y - d.y, -wallSize.z/4.0f}},
				cl_float3{{0.5, 0.5, 0.5}},
				1.0f/halfSize.y});
	lights.push_back(
			PointLight{
				cl_float3{{-wallSize.x/4.0f, halfSize.y - d.y, -wallSize.z/4.0f}},
				cl_float3{{0.5, 0.5, 0.5}},
				1.0f/halfSize.y});

	// Box
	if(numTiles == glm::ivec2(0, 0)) {
		Box::makeBoxInterior(tris, wallSize);
	} else if(numTiles == glm::ivec2(0, 1)) {
		Box::makePartialBoxInterior<1>(
				tris, wallSize, {{Box::Face::FRONT}},
				{{0, 1, 3, 4, 5}});
	} else if(numTiles == glm::ivec2(1, 0)) {
		Box::makePartialBoxInterior<1>(
				tris, wallSize, {{Box::Face::RIGHT}},
				{{0, 1, 2, 3, 5}});
	} else if(numTiles.x == 0) {
		Box::makePartialBoxInterior<2>(
				tris, wallSize, {{Box::Face::FRONT, Box::Face::BACK}},
				{{0, 1, 4, 5}});
	} else if(numTiles.y == 0) {
		Box::makePartialBoxInterior<2>(
				tris, wallSize, {{Box::Face::RIGHT, Box::Face::LEFT}});
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


	auto devCtx = std::make_shared<ClDeviceContext<DEVICE_TYPE>>();
	return std::make_shared<Scene<DEVICE_TYPE, LIGHT_MODEL>>(
					devCtx, spheres.begin(), spheres.end(),
					tris.begin(), tris.end(),
					lights.begin(), lights.end(),
					materials.begin(), materials.end());
}

}



#endif /* TILED_MIRROR_BOX_H_ */
