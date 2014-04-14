/*
 * geometry_array.h
 *
 *  Created on: Apr 12, 2014
 *      Author: francis
 */

#include <glm/glm.hpp>

#include <vector>

struct Triangle;
struct Sphere;
struct Quad;

#ifndef GEOMETRY_ARRAY_H_
#define GEOMETRY_ARRAY_H_

struct GeometryArray {
public:
	void addTriangle(const Triangle& t);

	template <typename Iter>
	void addTriangles(Iter begin, Iter end) {
		for(Iter i = begin; i != end; i++) {
			tris.push_back(*i);
		}
	}

	void addSphere(const Sphere& s);

	template <typename Iter>
	void addSpheres(Iter begin, Iter end) {
		for(Iter i = begin; i != end; i++) {
			spheres.push_back(*i);
		}
	}

	void addQuad(Quad& q);

	template <typename Iter>
	void addQuads(Iter begin, Iter end) {
		for(Iter i = begin; i != end; i++) {
			addQuad(*i);
		}
	}

	void transform(const glm::mat4& tx);

	size_t numSpheres() const;

	size_t numTriangles() const;

	const Triangle* triData();

	const Sphere* sphereData();

private:
	std::vector<Triangle> tris;
	std::vector<Sphere> spheres;
};

#endif /* GEOMETRY_ARRAY_H_ */
