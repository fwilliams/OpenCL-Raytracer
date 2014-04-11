/*
 * kaleidescope.h
 *
 *  Created on: Apr 10, 2014
 *      Author: francis
 */


#include <algorithm>

#include "renderer/data_types.h"

#ifndef KALEIDESCOPE_H_
#define KALEIDESCOPE_H_

class Kaleidescope {
public:

	void Kaleidescope(const std::vector<unsigned>& kpoints, float scale) {
		unsigned min_angle = std::max_element(kpoints.begin(), kpoints.end());

		// Use sine law to figure out sidelengths of triangle (or sidelegnths if *2222 or **)
		Triangle t;
		t.v1 = cl_float3{{-scale/2.0f, 0.0, scale/2.0f}};
		t.v2 = cl_float3{{ scale/2.0f, 0.0, scale/2.0f}};

		for(auto i = kpoints.begin(); i != kpoints.end()-1; i++) {
		}

		// Generate transformations for unique orientations
		// (At the same time, generate translation directions)

		// Tile space with translation directions
	}

private:
	std::vector<Triangle> triangles;
	// Need ceiling and floor material
	// Need way of putting lights and spheres in scene
};



#endif /* KALEIDESCOPE_H_ */
