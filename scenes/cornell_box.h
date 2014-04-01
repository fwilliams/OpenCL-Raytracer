/*
 * cornell_box_scene.h
 *
 *  Created on: Mar 26, 2014
 *      Author: francis
 */

#include <array>
#include <glm/glm.hpp>
#include "scene.h"
#include "box.h"

#ifndef CORNELL_BOX_SCENE_H_
#define CORNELL_BOX_SCENE_H_

namespace CornellBox {

template <LightModel LIGHT_MODEL>
constexpr std::array<Material<LIGHT_MODEL>, 8>  DefaultMaterials() {
	return std::array<Material<LIGHT_MODEL>, 8>{{
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.1, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.5, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.2, 0.2, 0.6}},
			cl_float3{{0.5, 0.5, 0.5}},
			100.0},
		Material<LIGHT_MODEL> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.6, 0.2, 0.2}},
			cl_float3{{0.5, 0.5, 0.5}},
			100.0}
	}};
}

template <>
constexpr std::array<Material<DIFFUSE>, 8>  DefaultMaterials<DIFFUSE>() {
	return std::array<Material<DIFFUSE>, 8>{{
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.1}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.1}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.1, 0.5}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.5}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.5, 0.1}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.5}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.2, 0.2, 0.6}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.6, 0.2, 0.2}}}
		}};
}

template<cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> buildCornellBox(
		const glm::vec3& wallSize,
		const std::array<Material<LIGHT_MODEL>, 8>& materials = DefaultMaterials<LIGHT_MODEL>()) {

	std::vector<Triangle> tris;
	std::vector<Sphere> spheres;
	std::vector<PointLight> lights;

	glm::vec3 halfSize = wallSize / 2.0f;

	Box::makeBoxInterior(tris, wallSize);

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
				cl_float3{{0.5, 0.5, 0.5}}});
	lights.push_back(
			PointLight{
				cl_float3{{-wallSize.x/4.0f, halfSize.y - d.y, wallSize.z/4.0f}},
				cl_float3{{0.5, 0.5, 0.5}}});
	lights.push_back(
			PointLight{
				cl_float3{{wallSize.x/4.0f, halfSize.y - d.y, -wallSize.z/4.0f}},
				cl_float3{{0.5, 0.5, 0.5}}});
	lights.push_back(
			PointLight{
				cl_float3{{-wallSize.x/4.0f, halfSize.y - d.y, -wallSize.z/4.0f}},
				cl_float3{{0.5, 0.5, 0.5}}});

	auto devCtx = std::make_shared<ClDeviceContext<DEVICE_TYPE>>();
	return std::make_shared<Scene<DEVICE_TYPE, LIGHT_MODEL>>(
					devCtx, spheres.begin(), spheres.end(),
					tris.begin(), tris.end(),
					lights.begin(), lights.end(),
					materials.begin(), materials.end());
}

}


#endif /* CORNELL_BOX_SCENE_H_ */
