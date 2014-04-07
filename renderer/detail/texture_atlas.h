/*
 * texture_atlas.h
 *
 *  Created on: Apr 4, 2014
 *      Author: francis
 */

#include <CL/cl.hpp>
#include <gli/gli.hpp>
#include <vector>
#include <algorithm>

#include "cl_device_context.h"
#include "texture_atlas_kd_tree.h"

#ifndef TEXTURE_ATLAS_H_
#define TEXTURE_ATLAS_H_

typedef unsigned TextureHdl;

class TextureAtlas {
public:
	cl::Image2D data;
	std::vector<glm::ivec4> texPositions;

	template <cl_device_type DEV_TYPE>
	static TextureAtlas makeTextureAtlas(ClDeviceContext<DEV_TYPE>& devCtx, TextureArray& texArray) {
		TextureAtlas texAtlas;
		glm::ivec2 size = computePositions(texArray, texAtlas.texPositions);

		texAtlas.data = cl::Image2D(
				devCtx.context, CL_MEM_READ_ONLY,
				cl::ImageFormat(CL_RGBA, CL_UNORM_INT8), size.x, size.y);

		TextureHdl index = 0;
		for(auto i : texAtlas.texPositions) {
			cl::size_t<3> origin;
			origin.push_back(i.x);
			origin.push_back(i.y);
			origin.push_back(0);
			cl::size_t<3> size;
			size.push_back(i.z);
			size.push_back(i.w);
			size.push_back(1);

			devCtx.commandQueue.enqueueWriteImage(
					texAtlas.data, true, origin, size, 0, 0, texArray[index++].data());
		}

		return texAtlas;
	}

private:
	static inline glm::ivec2 dims(const gli::storage& strg) {
		return glm::ivec2(strg.dimensions(0).x, strg.dimensions(0).y);
	}

	static inline unsigned area(const gli::storage& strg) {
		return dims(strg).x * dims(strg).y;
	};

	static inline glm::ivec2 computePositions(TextureArray& textures, std::vector<glm::ivec4>& texPosns) {
		// Aggregate all the texture Handles into a vector
		std::vector<TextureHdl> ids;
		for(unsigned i = 0; i < textures.size(); i++) {
			ids.push_back(i);
		}

		// Sort the textures by decreasing area
		std::sort(ids.begin(), ids.end(),
				[textures](TextureHdl a, TextureHdl b) mutable {
					return area(textures[a]) > area(textures[b]);
				});

		// Find the maximum height and width of all the textures
		glm::ivec2 maxDims(0);
		for(auto i : textures) {
			glm::ivec2 texDims = dims(i);
			if(texDims.x > maxDims.x) {
				maxDims.x = texDims.x;
			}
			if(texDims.y > maxDims.y) {
				maxDims.y = texDims.y;
			}
		}

		// Construct a KD tree with textures as leaf nodes
		TextureAtlasKDTree tree(maxDims);

		// Make sure we have enough space in the position
		texPosns.resize(ids.size());

		// Pack the texture positions
		for(auto i : ids) {
			texPosns[i] = tree.insert(i, dims(textures[i]));
		}

		// Return the size of the texture atlas rounded to the next power of 2
		return nextPowerOf2(tree.getSize());
	}

	static inline unsigned nextPowerOf2(unsigned x) {
	   x = x - 1;
	   x = x | (x >> 1);
	   x = x | (x >> 2);
	   x = x | (x >> 4);
	   x = x | (x >> 8);
	   x = x | (x >>16);
	   return x + 1;
	}

	static glm::vec2 nextPowerOf2(const glm::vec2& x) {
		return glm::vec2(nextPowerOf2(x.x), nextPowerOf2(x.y));
	}
};


#endif /* TEXTURE_ATLAS_H_ */
