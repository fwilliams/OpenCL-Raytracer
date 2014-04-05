/*
 * texture_atlas.h
 *
 *  Created on: Apr 4, 2014
 *      Author: francis
 */

#include <CL/cl.hpp>
#include <gli/gli.hpp>
#include <vector>
#include <map>
#include <algorithm>

#include "cl_device_context.h"

#ifndef TEXTURE_ATLAS_H_
#define TEXTURE_ATLAS_H_

class TextureAtlas {
public:
	unsigned createTexture(const gli::storage& texture) {
		textureData.push_back(texture);
		rects.push_back(glm::ivec4(0));
		return textureData.size()-1;
	}

	template <cl_device_type DEV_TYPE>
	cl::Image2D getTextureAtlas(ClDeviceContext<DEV_TYPE>& devCtx) {
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


private:
	std::vector<gli::storage> textureData;
	std::vector<glm::ivec4> rects;

	static inline glm::ivec2 dims(const gli::storage& strg) {
		return glm::ivec2(strg.dimensions(0).x, strg.dimensions(0).y);
	}

	static inline unsigned area(const gli::storage& strg) {
		return dims(strg).x * dims(strg).y;
	};


	inline glm::ivec2 computePositions() {
		std::vector<unsigned> ids;
		for(unsigned i = 0; i < textureData.size(); i++) {
			ids.push_back(i);
		}

		std::sort(ids.begin(), ids.end(), [this](unsigned a, unsigned b){ return area(textureData[a]) > area(textureData[b]); } );

		glm::ivec2 maxDims(0);
		for(auto i : textureData) {
			glm::ivec2 texDims = dims(i);
			if(texDims.x > maxDims.x) {
				maxDims.x = texDims.x;
			}
			if(texDims.y > maxDims.y) {
				maxDims.y = texDims.y;
			}
		}
		AtlasTree tree(maxDims);

		for(auto i : ids) {
			rects[i] = tree.insert(i, dims(textureData[i]));
		}

		return nextPowerOf2(tree.getSize());
	}

	unsigned nextPowerOf2(unsigned x) {
	   x = x - 1;
	   x = x | (x >> 1);
	   x = x | (x >> 2);
	   x = x | (x >> 4);
	   x = x | (x >> 8);
	   x = x | (x >>16);
	   return x + 1;
	}

	glm::vec2 nextPowerOf2(const glm::vec2& x) {
		return glm::vec2(nextPowerOf2(x.x), nextPowerOf2(x.y));
	}

	struct AtlasTree {
	private:
		struct node {
			static const unsigned NO_TEXTURE = -2;

			node() : rect(glm::ivec4(0)), left(nullptr), right(nullptr), texId(NO_TEXTURE) {}
			~node() {
				if(left != nullptr) {
					delete left;
				}
				if(right != nullptr) {
					delete right;
				}
			}

			glm::ivec4 rect;
			node* left;
			node* right;
			int texId;

			node* insert(unsigned tex, const glm::ivec2& size) {
				// If the left tree is nonempty, try to insert
				if(left != nullptr) {
					node* res = nullptr;
					res = left->insert(tex, size);
					if(res != nullptr) {
						return res;
					}
				}
				// If we couldn't insert into the left tree and the right tree is nonempty, try to insert
				if(right != nullptr) {
					return right->insert(tex, size);
				}

				// If we are a leaf node and already have a texture return nothing
				if(texId != NO_TEXTURE) {
					return nullptr;
				}

				// If we're a leaf node and the new texture doesn't fit, return nothing
				if(rect.z < size.x || rect.w < size.y) {
					return nullptr;
				}

				// If we're a leaf node, and the new texture fits perfectly, add the texture, return this node
				if(rect.z == size.x && rect.w == size.y) {
					texId = tex;
					return this;
				}

				// If we don't fit perfectly, create a new child that does and a new child for the empty space
				left = new node();
				right = new node();

				unsigned dw = rect.z - size.x;
				unsigned dh = rect.w - size.y;

				if(dw > dh) {
					left->rect = glm::ivec4(rect.x, rect.y, size.x, rect.w);
					right->rect = glm::ivec4(rect.x+size.x, rect.y, rect.z-size.x, rect.w);
				} else {
					left->rect = glm::ivec4(rect.x, rect.y, rect.z, size.y);
					right->rect = glm::ivec4(rect.x, rect.y+size.y, rect.z, rect.w-size.y);
				}

				return left->insert(tex, size);
			}

		};

		node* tree = nullptr;
		glm::ivec2 maxDims;
	public:

		AtlasTree(glm::ivec2 maxDims) : maxDims(maxDims) {}

		~AtlasTree() {
			delete tree;
		}

		glm::ivec2 getSize() {
			return glm::ivec2(tree->rect.z, tree->rect.w);
		}

		glm::ivec4 insert(unsigned texId, glm::ivec2 size) {
			// If the tree hasn't been initialized, initialize it with the maximum possible texture size
			if(tree == nullptr) {
				tree = new node();
				tree->rect = glm::ivec4(0, 0, maxDims.x, maxDims.y);
			}

			// Try to insert the texture
			node* newNode = tree->insert(texId, size);

			// If the texture doesn't fit, create a new tree containing the old one and an empty tree
			// the same size as the old one. Note that there's an invariant here that insertions will
			// be performed in sorted order from largest to smallest.
			// The resizing dimension alternates between left and right.
			if(newNode == nullptr) {
				node* newRoot = new node();
				if(tree->rect.z <= tree->rect.w) {
					newRoot->rect = tree->rect;
					newRoot->rect.z *= 2;

					newRoot->left = tree;
					newRoot->right = new node();
					newRoot->right->rect = tree->rect;
					newRoot->right->rect.x += tree->rect.z;
				} else {
					newRoot->rect = tree->rect;
					newRoot->rect.w *= 2;

					newRoot->left = tree;
					newRoot->right = new node();
					newRoot->right->rect = tree->rect;
					newRoot->right->rect.y += tree->rect.w;
				}
				// This is guaranteed to succeed because of the invariant that textures are inserted in
				// order of decreasing size, the first subtree was initialized with the largest possible dimensions.
				// Therefore an empty tree can contain any of the inserted textures
				newNode = newRoot->right->insert(texId, size);
				tree = newRoot;
			}

			return newNode->rect;
		}
	};
};


#endif /* TEXTURE_ATLAS_H_ */
