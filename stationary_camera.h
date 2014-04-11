/*
 * stationary_camera.h
 *
 *  Created on: Apr 10, 2014
 *      Author: francis
 */

#ifndef STATIONARY_CAMERA_H_
#define STATIONARY_CAMERA_H_

class StationaryCamera {
	StationaryCamera() {
	}

	const glm::mat4& getViewMatrix() {
		return identity;
	}

	bool isMoving() {
		return false;
	}

	void update(SDL_Event& evt) {
	}
private:
	glm::mat4 identity;
};



#endif /* STATIONARY_CAMERA_H_ */
