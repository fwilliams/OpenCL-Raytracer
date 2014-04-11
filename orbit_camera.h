/*
 * orbit_camera.h
 *
 *  Created on: Apr 10, 2014
 *      Author: francis
 */

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef ORBIT_CAMERA_H_
#define ORBIT_CAMERA_H_

class OrbitCamera {
public:
	OrbitCamera(glm::vec3 vel, float radius, glm::vec2 radiusRange, glm::vec3 origin = glm::vec3(0.0)) :
		viewMat(glm::mat4()), vel(vel), origin(origin), radiusRange(radiusRange),
		lastMousePos(glm::vec2(0.0)), camAngle(glm::vec2(0.0)), radius(radius), mouseDragging(false) {
	}

	const glm::mat4& getViewMatrix() {
		glm::mat4 tx = glm::rotate(glm::mat4(1.0), camAngle.x, glm::vec3(0.0, 1.0, 0.0));
		tx = glm::rotate(tx, camAngle.y, glm::vec3(1.0, 0.0, 0.0));

		viewMat = glm::inverse(
				  glm::lookAt(
						glm::vec3(tx * glm::vec4(0.0, 0.0, radius, 1.0)),
						 origin, glm::vec3(0.0, 1.0, 0.0)));
		return viewMat;
	}

	bool isMoving() {
		return mouseDragging;
	}

	void update(SDL_Event& evt) {
		if(evt.type == SDL_MOUSEBUTTONDOWN) {
			lastMousePos = glm::vec2(evt.button.x, evt.button.y);
			mouseDragging = true;
		} else if(evt.type == SDL_MOUSEBUTTONUP) {
			mouseDragging = false;
		} else if(evt.type == SDL_MOUSEMOTION) {
			if(mouseDragging) {
				glm::vec2 mousePos(evt.button.x, evt.button.y);
				camAngle += (mousePos - lastMousePos) * glm::vec2(vel);
				lastMousePos = mousePos;
			}
		} else if(evt.type == SDL_MOUSEWHEEL) {

			radius -= evt.wheel.y * vel.z;
		}
	}

private:
	glm::mat4 viewMat;
	glm::vec3 vel;
	glm::vec3 origin;
	glm::vec2 radiusRange;
	glm::vec2 lastMousePos;
	glm::vec2 camAngle;
	float radius;
	bool mouseDragging;
};



#endif /* ORBIT_CAMERA_H_ */
