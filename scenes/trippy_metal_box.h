/*
 * trippy_metal_box.h
 *
 *  Created on: Mar 27, 2014
 *      Author: francis
 */

#include "scene.h"

#ifndef TRIPPY_METAL_BOX_H_
#define TRIPPY_METAL_BOX_H_

namespace TrippyMetalBox {

template <LightModel LIGHT_MODEL>
constexpr std::array<Material<LIGHT_MODEL>, 3>  DefaultMaterials();

template <>
constexpr std::array<Material<BLINN_PHONG>, 3>  DefaultMaterials<BLINN_PHONG>() {
	return std::array<Material<BLINN_PHONG>, 3>{{
		Material<BLINN_PHONG> {
			cl_float3{{0.05, 0.05, 0.05}},
			cl_float3{{0.4, 0.4, 0.6}},
			cl_float3{{0.5, 0.5, 0.5}},
			1000.0},
		Material<BLINN_PHONG> {
			cl_float3{{0.05, 0.05, 0.05}},
			cl_float3{{0.6, 0.4, 0.4}},
			cl_float3{{0.5, 0.5, 0.5}},
			1000.0},
		Material<BLINN_PHONG> {
			cl_float3{{0.9, 0.9, 0.9}},
			cl_float3{{0.1, 0.1, 0.1}},
			cl_float3{{0.5, 0.5, 0.5}},
			10000.0}
	}};
}

template <>
constexpr std::array<Material<PHONG>, 3>  DefaultMaterials<PHONG>() {
	return std::array<Material<PHONG>, 3>{{
		Material<PHONG> {
			cl_float3{{0.05, 0.05, 0.05}},
			cl_float3{{0.4, 0.4, 0.6}},
			cl_float3{{0.5, 0.5, 0.5}},
			1000.0},
		Material<PHONG> {
			cl_float3{{0.05, 0.05, 0.05}},
			cl_float3{{0.6, 0.4, 0.4}},
			cl_float3{{0.5, 0.5, 0.5}},
			1000.0},
		Material<PHONG> {
			cl_float3{{0.9, 0.9, 0.9}},
			cl_float3{{0.1, 0.1, 0.1}},
			cl_float3{{0.5, 0.5, 0.5}},
			10000.0}
	}};
}

template <>
constexpr std::array<Material<DIFFUSE>, 3>  DefaultMaterials<DIFFUSE>() {
	return std::array<Material<DIFFUSE>, 3>{{
		Material<DIFFUSE> {
			cl_float3{{0.05, 0.05, 0.05}},
			cl_float3{{0.4, 0.4, 0.6}}},
		Material<DIFFUSE> {
			cl_float3{{0.05, 0.05, 0.05}},
			cl_float3{{0.6, 0.4, 0.4}}},
		Material<DIFFUSE> {
			cl_float3{{0.9, 0.9, 0.9}},
			cl_float3{{0.1, 0.1, 0.1}}}
	}};
}

template<cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL>> buildTrippyBoxScene(
		const glm::vec2& wallSize, float wallSep,
		const std::array<Material<LIGHT_MODEL>, 3>& materials = DefaultMaterials<LIGHT_MODEL>()) {

	std::vector<Triangle> tris;
	std::vector<Sphere> spheres;
	std::vector<PointLight> lights;

	// Tweak these for fun!
	const float breakOffset  = 0.0f; //static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	const unsigned trisX = 1;
	const unsigned trisY = 1;

	float halfSeperation = wallSep/2.0;
	glm::vec2 halfSize = wallSize/2.0f;

	float dX = wallSize.x/trisX;
	float dY = wallSize.y/trisY;

	for(unsigned i = 0; i < trisX; i++) {
		for(unsigned j = 0; j < trisY; j++) {
			// Top plane
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     halfSeperation+breakOffset, -halfSize.y+j*dY}},
					cl_float3{{-halfSize.x+i*dX,     halfSeperation+breakOffset, -halfSize.y+(j+1)*dY}},
					cl_float3{{-halfSize.x+(i+1)*dX, halfSeperation+breakOffset, -halfSize.y+(j+1)*dY}},
					2});
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     halfSeperation+breakOffset, -halfSize.y+j*dY}},
					cl_float3{{-halfSize.x+(i+1)*dX, halfSeperation+breakOffset, -halfSize.y+(j+1)*dY}},
					cl_float3{{-halfSize.x+(i+1)*dX, halfSeperation+breakOffset, -halfSize.y+j*dY}},
					2});

			// Bottom plane
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     -halfSeperation-breakOffset, -halfSize.y+j*dY}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSeperation-breakOffset, -halfSize.y+(j+1)*dY}},
					cl_float3{{-halfSize.x+i*dX,     -halfSeperation-breakOffset, -halfSize.y+(j+1)*dY}},
					2});
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     -halfSeperation-breakOffset, -halfSize.y+j*dY}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSeperation-breakOffset, -halfSize.y+j*dY}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSeperation-breakOffset, -halfSize.y+(j+1)*dY}},
					2});

			// Front Plane
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY, 	  -halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+i*dX,     -halfSize.y+(j+1)*dY, -halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY, -halfSeperation+breakOffset}},
					2});
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY,    -halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY,-halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+j*dY,    -halfSeperation+breakOffset}},
					2});

			// Back Plane
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY, 	  halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+i*dX,     -halfSize.y+(j+1)*dY, halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY, halfSeperation+breakOffset}},
					2});
			tris.push_back(
				Triangle{
					cl_float3{{-halfSize.x+i*dX,     -halfSize.y+j*dY,    halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY,halfSeperation+breakOffset}},
					cl_float3{{-halfSize.x+(i+1)*dX, -halfSize.y+j*dY,    halfSeperation+breakOffset}},
					2});

			// Right Plane
			tris.push_back(
				Triangle{
					cl_float3{{halfSeperation+breakOffset, -halfSize.x+i*dX,     -halfSize.y+j*dY}},
					cl_float3{{halfSeperation+breakOffset, -halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY}},
					cl_float3{{halfSeperation+breakOffset, -halfSize.x+i*dX,     -halfSize.y+(j+1)*dY}},
					2});
			tris.push_back(
				Triangle{
					cl_float3{{halfSeperation+breakOffset, -halfSize.x+i*dX,     -halfSize.y+j*dY}},
					cl_float3{{halfSeperation+breakOffset, -halfSize.x+(i+1)*dX, -halfSize.y+j*dY}},
					cl_float3{{halfSeperation+breakOffset, -halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY}},
					2});

			// Left Plane
			tris.push_back(
				Triangle{
					cl_float3{{-halfSeperation+breakOffset, -halfSize.x+i*dX,     -halfSize.y+j*dY}},
					cl_float3{{-halfSeperation+breakOffset, -halfSize.x+i*dX,     -halfSize.y+(j+1)*dY}},
					cl_float3{{-halfSeperation+breakOffset, -halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY}},
					2});
			tris.push_back(
				Triangle{
					cl_float3{{-halfSeperation+breakOffset, -halfSize.x+i*dX,     -halfSize.y+j*dY}},
					cl_float3{{-halfSeperation+breakOffset, -halfSize.x+(i+1)*dX, -halfSize.y+(j+1)*dY}},
					cl_float3{{-halfSeperation+breakOffset, -halfSize.x+(i+1)*dX, -halfSize.y+j*dY}},
					2});
		}
	}

	// Spheres
	float radius = std::min(wallSize.x, wallSize.y)/10.0f;

	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/4.0f, breakOffset, -wallSize.y/4.0f}}, 0});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/4.0f, breakOffset, -wallSize.y/4.0f}}, 1});
	spheres.push_back(Sphere{radius, cl_float3{{-wallSize.x/4.0f, breakOffset,  wallSize.y/4.0f}}, 1});
	spheres.push_back(Sphere{radius, cl_float3{{ wallSize.x/4.0f, breakOffset,  wallSize.y/4.0f}}, 0});

	// Lights
	lights.push_back(
			PointLight{
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.5, 0.5, 0.5}}});
	lights.push_back(
			PointLight{
				cl_float3{{0.0, wallSize.y / 4.0f, 0.0}},
				cl_float3{{0.5, 0.5, 0.5}}});

	auto devCtx = std::make_shared<ClDeviceContext<DEVICE_TYPE>>();
	return std::make_shared<Scene<DEVICE_TYPE, LIGHT_MODEL>>(
					devCtx, spheres.begin(), spheres.end(),
					tris.begin(), tris.end(),
					lights.begin(), lights.end(),
					materials.begin(), materials.end());
}

}
#endif /* TRIPPY_METAL_BOX_H_ */
