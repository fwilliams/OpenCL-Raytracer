/*
 * texture_atlas.h
 *
 *  Created on: Apr 4, 2014
 *      Author: francis
 */

#include <gli/gli.hpp>
#include <CL/cl.hpp>
#include <vector>
#include <map>
#include <algorithm>

#include "cl_device_context.h"

#ifndef TEXTURE_ATLAS_H_
#define TEXTURE_ATLAS_H_

class TextureAtlas {
public:
	// Add a texture to the atlas and return a textureId
	unsigned createTexture(const gli::storage& texture) {
		textureData.push_back(texture);
		rects.push_back(glm::ivec4(0));
		return 0;
	}

	template <cl_device_type DEV_TYPE>
	cl::Image2D packAtlas(ClDeviceContext<DEV_TYPE>& devCtx) {
		glm::ivec2 atlasSize = computePositions();

		cl::Image2D	textureAtlas = cl::Image2D(
				devCtx.context, CL_MEM_READ_ONLY,
				cl::ImageFormat(CL_RGBA, CL_UNORM_INT8), atlasSize.x, atlasSize.y);

		unsigned index = 0;
		for(auto i : rects) {
			cl::size_t<3> origin;
			origin.push_back(i.x);
			origin.push_back(i.y);
			origin.push_back(0);
			cl::size_t<3> size;
			size.push_back(i.z);
			size.push_back(i.w);
			size.push_back(1);

			devCtx.commandQueue.enqueueWriteImage(
					textureAtlas, true, origin, size, 0, 0, textureData[index++].data());
		}

		return textureAtlas;
	}

	glm::ivec2 computePositions() {

		struct node {
			glm::ivec4 rect;
			unsigned left, right;
		};

		std::vector<unsigned> ids;
		for(unsigned i = 0; i < textureData.size(); i++) {
			ids.push_back(i);
		}

		std::sort(ids.begin(), ids.end(), [this](unsigned a, unsigned b){ return area(textureData[a]) > area(textureData[b]); } );

		gli::storage* largestTexture = &textureData[ids[0]];

		float largestDim = glm::max(static_cast<float>(width(*largestTexture)), static_cast<float>(height(*largestTexture)));

		unsigned dim = static_cast<unsigned>(glm::pow(2.0f, glm::ceil(glm::log(largestDim)/glm::log(2.0f))));

		glm::ivec4 currentSize(0, 0, dim, dim);
		unsigned scanlineHeight = height(*largestTexture);

		for(auto i : ids) {
			std::cout << i << std::endl;
			if((currentSize.z - currentSize.x) < width(textureData[i])) {
				if(currentSize.z == currentSize.w) {
					currentSize.z *= 2;
				} else {
					currentSize.w *= 2;

					currentSize.x = 0;
					currentSize.y = currentSize.y+scanlineHeight;

					scanlineHeight += height(textureData[i]);
				}
			}
			rects[i] = glm::ivec4(currentSize.x, currentSize.y, width(textureData[i]), height(textureData[i]));

			std::cout << "inserted texture " << i <<
					" with size (" << width(textureData[i]) << ", " << height(textureData[i]) <<
					") at position (" << currentSize.x << ", " << currentSize.y << ")." << std::endl;
			std::cout << "atlas size is now (" << currentSize.z << ", " << currentSize.w << ")" << std::endl;

			currentSize.x += width(textureData[i]);
		}

		return glm::ivec2(currentSize.z, currentSize.w);
	}

private:
	static unsigned width(const gli::storage& strg) {
		return strg.dimensions(0).x;
	};

	static unsigned height(const gli::storage& strg) {
		return strg.dimensions(0).y;
	};

	static unsigned area(const gli::storage& strg) {
		return width(strg) * height(strg);
	};

	std::vector<gli::storage> textureData;
	std::vector<glm::ivec4> rects;
};


#endif /* TEXTURE_ATLAS_H_ */
