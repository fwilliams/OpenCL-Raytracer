/*
 * scene.h
 *
 *  Created on: Apr 13, 2014
 *      Author: francis
 */

#include <vector>

#include "data_types.h"
#include "geometry_array.h"
#include "texture_array.h"

#ifndef SCENE_H_
#define SCENE_H_

template <BRDFType BRDF>
struct Scene {
	GeometryArray geometry;
	TextureArray textures;
	std::vector<PointLight> lights;
	std::vector<Material<BRDF>> materials;
};



#endif /* SCENE_H_ */
