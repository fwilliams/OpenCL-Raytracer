/*
 * data_types.cpp
 *
 *  Created on: Apr 13, 2014
 *      Author: francis
 */

#include "data_types.h"

void Sphere::transform(const glm::mat4& tx) {
	glm::vec4 newPos(position.s[0], position.s[1], position.s[2], 1.0);
	newPos = tx * newPos;
	position = cl_float3{{newPos.x, newPos.y, newPos.z}};
}

void Triangle::transform(const glm::mat4& tx) {
	glm::vec4 newV;

	newV = tx * glm::vec4(vp1.s[0], vp1.s[1], vp1.s[2], 1.0);
	vp1 = {{newV.x, newV.y, newV.z}};

	newV = tx * glm::vec4(vp2.s[0], vp2.s[1], vp2.s[2], 1.0);
	vp2 = {{newV.x, newV.y, newV.z}};

	newV = tx * glm::vec4(vp3.s[0], vp3.s[1], vp3.s[2], 1.0);
	vp3 = {{newV.x, newV.y, newV.z}};

	glm::vec3 newN;
	newN = glm::transpose(glm::inverse(glm::mat3(tx))) *
			glm::vec3(normal.s[0], normal.s[1], normal.s[2]);
	normal = {{newN.x, newN.y, newN.z}};
}

void Quad::transform(const glm::mat4& tx) {
	glm::vec4 newV;

	newV = tx * glm::vec4(vp1.s[0], vp1.s[1], vp1.s[2], 1.0);
	vp1 = {{newV.x, newV.y, newV.z}};

	newV = tx * glm::vec4(vp2.s[0], vp2.s[1], vp2.s[2], 1.0);
	vp2 = {{newV.x, newV.y, newV.z}};

	newV = tx * glm::vec4(vp3.s[0], vp3.s[1], vp3.s[2], 1.0);
	vp3 = {{newV.x, newV.y, newV.z}};

	newV = tx * glm::vec4(vp4.s[0], vp4.s[1], vp4.s[2], 1.0);
	vp4 = {{newV.x, newV.y, newV.z}};

	glm::vec3 newN;
	newN = glm::transpose(glm::inverse(glm::mat3(tx))) *
			glm::vec3(normal.s[0], normal.s[1], normal.s[2]);
	normal = {{newN.x, newN.y, newN.z}};
}

void PointLight::transform(const glm::mat4& tx) {
	glm::vec4 newPos;
	newPos = tx * glm::vec4(position.s[0], position.s[1], position.s[2], 1.0);
	position = {{newPos.x, newPos.y, newPos.z}};
}



