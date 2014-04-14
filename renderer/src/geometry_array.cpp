/*
 * geometry_array.cpp
 *
 *  Created on: Apr 13, 2014
 *      Author: francis
 */

#include "geometry_array.h"
#include "data_types.h"

void GeometryArray::addTriangle(const Triangle& t) {
	tris.push_back(t);
}

void GeometryArray::addSphere(const Sphere& s) {
	spheres.push_back(s);
}

void GeometryArray::addQuad(Quad& q) {
	tris.push_back(
			Triangle{
				q.vp1, q.vp2, q.vp3,
				q.vt1, q.vt2, q.vt3,
				q.normal, q.materialId
			});
	tris.push_back(
			Triangle{
				q.vp3, q.vp4, q.vp1,
				q.vt3, q.vt4, q.vt1,
				q.normal, q.materialId
			});
}

void GeometryArray::transform(const glm::mat4& tx) {
	for(auto i : tris) {
		i.transform(tx);
	}

	for(auto i : spheres) {
		i.transform(tx);
	}
}

size_t GeometryArray::numSpheres() const {
	return spheres.size();
}

size_t GeometryArray::numTriangles() const {
	return tris.size();
}

const Triangle* GeometryArray::GeometryArray::triData() {
	return tris.data();
}

const Sphere* GeometryArray::sphereData() {
	return spheres.data();
}



