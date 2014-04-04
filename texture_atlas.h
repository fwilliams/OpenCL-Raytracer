/*
 * texture_atlas.h
 *
 *  Created on: Apr 4, 2014
 *      Author: francis
 */

#include <gli/gli.hpp>
#include <CL/cl.hpp>
#include <string>
#include <vector>
#include <algorithm>

#ifndef TEXTURE_ATLAS_H_
#define TEXTURE_ATLAS_H_

class TextureAtlas {
public:
	// Add a texture to the atlas and return a textureId
	unsigned createTexture(gli::storage& texture) {
		textureData.push_back(texture);
	}

	cl_float4*  getAtlasDataPtr(cl_float2& size) {
		// TODO: Pack atlas
		return nullptr;
	}
private:
	std::vector<gli::storage> textureData;
};


#endif /* TEXTURE_ATLAS_H_ */
