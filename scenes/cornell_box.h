/*
 * cornell_box_scene.h
 *
 *  Created on: Mar 26, 2014
 *      Author: francis
 */

#include <glm/glm.hpp>
#include <array>
#include "scene.h"

#ifndef CORNELL_BOX_SCENE_H_
#define CORNELL_BOX_SCENE_H_

namespace CornellBox {

template <LightModel LIGHT_MODEL>
constexpr std::array<Material<LIGHT_MODEL>, 6>  DefaultMaterials();

template <>
constexpr std::array<Material<BLINN_PHONG>, 6>  DefaultMaterials<BLINN_PHONG>() {
	return std::array<Material<BLINN_PHONG>, 6>{
		Material<BLINN_PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<BLINN_PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<BLINN_PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.1, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<BLINN_PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<BLINN_PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.5, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<BLINN_PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0}
		};
}

template <>
constexpr std::array<Material<PHONG>, 6>  DefaultMaterials<PHONG>() {
	return std::array<Material<PHONG>, 6>{
		Material<PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.1, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.5, 0.1}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0},
		Material<PHONG> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.5}},
			cl_float3{{0.0, 0.0, 0.0}},
			1.0}
		};
}

template <>
constexpr std::array<Material<DIFFUSE>, 6>  DefaultMaterials<DIFFUSE>() {
	return std::array<Material<DIFFUSE>, 6>{
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.1}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.1}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.1, 0.5}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.1, 0.5}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.5, 0.5, 0.1}}},
		Material<DIFFUSE> {
			cl_float3{{0.0, 0.0, 0.0}},
			cl_float3{{0.1, 0.5, 0.5}}}
		};
}

template<cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> buildCornellBox(
		const glm::vec3& wallSize,
		const std::array<Material<LIGHT_MODEL>, 6>& materials = DefaultMaterials<LIGHT_MODEL>()) {

	std::vector<Triangle> tris;
	std::vector<Sphere> spheres;
	std::vector<PointLight> lights;

	// Tweak these for fun!
	const float breakOffset  = 0.0f; //static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
	const unsigned trisX = 1;
	const unsigned trisY = 1;
	const unsigned trisZ = 1;

	glm::vec3 halfSize = wallSize/2.0f;

	float dX = wallSize.x/trisX;
	float dY = wallSize.y/trisY;
	float dZ = wallSize.y/trisZ;

	for(unsigned i = 0; i < trisX; i++) {
		for(unsigned j = 0; j < trisY; j++) {
			for(unsigned k = 0; k < trisZ; k++) {
				// Top plane
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     halfSize.y+breakOffset, -halfSize.z+k*dZ}},
						cl_float3{{-halfSize.x+i*dX,     halfSize.y+breakOffset, -halfSize.z+(k+1)*dZ}},
						cl_float3{{-halfSize.x+(i+1)*dX, halfSize.y+breakOffset, -halfSize.z+(k+1)*dZ}},
						0});
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     halfSize.y+breakOffset, -halfSize.z+k*dZ}},
						cl_float3{{-halfSize.x+(i+1)*dX, halfSize.y+breakOffset, -halfSize.z+(k+1)*dZ}},
						cl_float3{{-halfSize.x+(i+1)*dX, halfSize.y+breakOffset, -halfSize.z+k*dZ}},
						0});

				// Bottom plane
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y-breakOffset, -halfSize.z+k*dZ}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y-breakOffset, -halfSize.z+(k+1)*dZ}},
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y-breakOffset, -halfSize.z+(k+1)*dZ}},
						1});
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y-breakOffset, -halfSize.z+k*dZ}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y-breakOffset, -halfSize.z+k*dZ}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y-breakOffset, -halfSize.z+(k+1)*dZ}},
						1});

				// Front Plane
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY, 	   -halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y+(j+1)*dY, -halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY, -halfSize.z+breakOffset}},
						2});
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY,    -halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY,-halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+j*dY,    -halfSize.z+breakOffset}},
						2});

				// Back Plane
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY, 	   halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y+(j+1)*dY, halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY, halfSize.z+breakOffset}},
						3});
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY,     halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY, halfSize.z+breakOffset}},
						cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+j*dY,     halfSize.z+breakOffset}},
						3});

				// Right Plane
				tris.push_back(
					Triangle{
						cl_float3{{halfSize.x+breakOffset, -halfSize.y+j*dY,     -halfSize.z+k*dZ}},
						cl_float3{{halfSize.x+breakOffset, -halfSize.y+(j+1)*dY, -halfSize.z+(k+1)*dZ}},
						cl_float3{{halfSize.x+breakOffset, -halfSize.y+j*dY,     -halfSize.z+(k+1)*dZ}},
						4});
				tris.push_back(
					Triangle{
						cl_float3{{halfSize.x+breakOffset, -halfSize.y+j*dY,     -halfSize.z+k*dZ}},
						cl_float3{{halfSize.x+breakOffset, -halfSize.y+(j+1)*dY, -halfSize.z+k*dZ}},
						cl_float3{{halfSize.x+breakOffset, -halfSize.y+(j+1)*dY, -halfSize.z+(k+1)*dZ}},
						4});

				// Left Plane
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+breakOffset, -halfSize.y+j*dY,     -halfSize.z+k*dZ}},
						cl_float3{{-halfSize.x+breakOffset, -halfSize.y+j*dY,     -halfSize.z+(k+1)*dZ}},
						cl_float3{{-halfSize.x+breakOffset, -halfSize.y+(j+1)*dY, -halfSize.z+(k+1)*dZ}},
						5});
				tris.push_back(
					Triangle{
						cl_float3{{-halfSize.x+breakOffset, -halfSize.y+j*dY,     -halfSize.z+k*dZ}},
						cl_float3{{-halfSize.x+breakOffset, -halfSize.y+(j+1)*dY, -halfSize.z+(k+1)*dZ}},
						cl_float3{{-halfSize.x+breakOffset, -halfSize.y+(j+1)*dY, -halfSize.z+k*dZ}},
						5});
			}
		}
	}

	// Spheres
	float radius = std::min(std::min(wallSize.x, wallSize.y), wallSize.z) /10.0f;

	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/4.0f, -halfSize.y+radius+breakOffset, -wallSize.z/4.0f}}, 0});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/4.0f, -halfSize.y+radius+breakOffset, -wallSize.z/4.0f}}, 1});
	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/4.0f, -halfSize.y+radius+breakOffset,  wallSize.z/4.0f}}, 1});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/4.0f, -halfSize.y+radius+breakOffset,  wallSize.z/4.0f}}, 0});

	// Lights
	lights.push_back(
			PointLight{
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.5, 0.5, 0.5}}});
	lights.push_back(
			PointLight{
				cl_float3{{0.0, wallSize.y / 2.0f, 0.0}},
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