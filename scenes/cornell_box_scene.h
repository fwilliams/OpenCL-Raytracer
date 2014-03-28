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

enum Wall { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };

struct WallMirror {
	glm::vec2 size;			// Size with respect to the wall
	glm::vec2 translation;	// Offset from center
	Wall wall;				// Which wall this mirror is on
};

template <cl_device_type DEVICE_TYPE, LightModel LIGHT_MODEL>
std::shared_ptr<Scene<DEVICE_TYPE, LIGHT_MODEL, std::vector>> makeCornellBox(
		const glm::vec3& dims, const glm::vec3& lightingIntensity,
		const std::array<Material<LIGHT_MODEL>, 6>& wallMats) {

	const float epsFrac = 1000.0f;
	glm::vec3 wallEps = dims / epsFrac;
	glm::vec3 halfSep = dims / 2.0f;
	glm::vec3 lightSep = halfSep / 2.0f;
	glm::vec3 intensity = lightingIntensity / 4.0f;

	std::vector<Triangle> tris(12);
	std::vector<PointLight> lights(4);
	std::vector<Sphere> spheres(0);

	// Back wall
	tris.push_back({
		{{-halfSep.x, -halfSep.y, -halfSep.z}},
		{{ halfSep.x, -halfSep.y, -halfSep.z}},
		{{ halfSep.x,  halfSep.y, -halfSep.z}},
		0});
//	tris.push_back({
//		{{-halfSep.x, -halfSep.y, -halfSep.z}},
//		{{ halfSep.x,  halfSep.y, -halfSep.z}},
//		{{ halfSep.x, -halfSep.y, -halfSep.z}},
//		0});

		// Lights
		lights.push_back(
				PointLight{
					cl_float3{{-lightSep.x, halfSep.y-wallEps.y, -lightSep.y}},
					cl_float3{{intensity.x, intensity.y, intensity.z}}});
		lights.push_back(
				PointLight{
					cl_float3{{ lightSep.x, halfSep.y-wallEps.y,  lightSep.y}},
					cl_float3{{intensity.x, intensity.y, intensity.z}}});
		lights.push_back(
				PointLight{
					cl_float3{{-lightSep.x, halfSep.y-wallEps.y,  lightSep.y}},
					cl_float3{{intensity.x, intensity.y, intensity.z}}});
		lights.push_back(
				PointLight{
					cl_float3{{ lightSep.x, halfSep.y-wallEps.y, -lightSep.y}},
					cl_float3{{intensity.x, intensity.y, intensity.z}}});


		auto devCtx = std::make_shared<ClDeviceContext<CL_DEVICE_TYPE_GPU>>();
		return std::make_shared<Scene<CL_DEVICE_TYPE_GPU, BLINN_PHONG>>(
						devCtx, spheres.begin(), spheres.end(),
						tris.begin(), tris.end(),
						lights.begin(), lights.end(),
						wallMats.begin(), wallMats.end());
}

}


#endif /* CORNELL_BOX_SCENE_H_ */
