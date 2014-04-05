/*
 * texture_atlas.h
 *
 *  Created on: Apr 4, 2014
 *      Author: francis
 */

#include <gli/gli.hpp>
#include <glm/gtx/string_cast.hpp>

#include <CL/cl.hpp>
#include <vector>
#include <map>
#include <algorithm>

#include "cl_device_context.h"

#ifndef TEXTURE_ATLAS_H_
#define TEXTURE_ATLAS_H_

struct SpaceTree {
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
			std::cout << "Insert function!" << std::endl;

			if(left != nullptr) {
				std::cout << "Left tree ain't empty. Insert here!" << std::endl;
				node* res = nullptr;
				res = left->insert(tex, size);
				if(res != nullptr) {
					return res;
				}
			}
			if(right != nullptr) {
				std::cout << "Right tree ain't empty. Insert here!" << std::endl;
				return right->insert(tex, size);
			}

			if(texId != NO_TEXTURE) {
				std::cout << "There's already a texture here. fuckit.jpg!" << std::endl;
				return nullptr;
			}

			if(rect.z < size.x || rect.w < size.y) {
				std::cout << "This hole is too small. fuckit.jpg!" << std::endl;
				return nullptr;
			}

			if(rect.z == size.x && rect.w == size.y) {
				std::cout << "Perfect fit!. feelsgoodman!" << std::endl;
				texId = tex;
				return this;
			}

			std::cout << "Inserting: " << std::endl;
			std::cout << "    " << glm::to_string(rect) << std::endl;
			std::cout << "    " << glm::to_string(size) << std::endl;
			left = new node();
			right = new node();

			unsigned dw = rect.z - size.x;
			unsigned dh = rect.w - size.y;

			if(dw > dh) {
				std::cout << "dw > dh" << std::endl;
				left->rect = glm::ivec4(rect.x, rect.y, size.x, rect.w);
				right->rect = glm::ivec4(rect.x+size.x, rect.y, rect.z-size.x, rect.w);
			} else {
				std::cout << "dw <= dh" << std::endl;
				left->rect = glm::ivec4(rect.x, rect.y, rect.z, size.y);
				right->rect = glm::ivec4(rect.x, rect.y+size.y, rect.z, rect.w-size.y);
			}

			return left->insert(tex, size);
		}

	};

	~SpaceTree() {
		delete tree;
	}

	node* tree = nullptr;
	glm::ivec4 insert(unsigned texId, glm::ivec2 size) {
		if(tree == nullptr) {
			std::cout << "Initialized dat new tree! " << glm::to_string(size) << std::endl;
			tree = new node();
			tree->rect = glm::ivec4(0, 0, size.x, size.y);
		}

		std::cout << "===== TREE HAS SIZE (" << tree->rect.z << ", " << tree->rect.w << ") =====" << std::endl;

		node* newNode = tree->insert(texId, size);
		if(newNode == nullptr) {
			std::cout << "Didn't fit. Time to enlarge this tree!" << std::endl;
			node* newRoot = new node();
			if(tree->rect.z <= tree->rect.w) {
				newRoot->rect = tree->rect;
				newRoot->rect.z *= 2;

				newRoot->left = tree;
				newRoot->right = new node();
				newRoot->right->rect = tree->rect;
				newRoot->right->rect.x += tree->rect.z;
				std::cout << "Make it wider! " << glm::to_string(newRoot->rect) << std::endl;
			} else {
				newRoot->rect = tree->rect;
				newRoot->rect.w *= 2;

				newRoot->left = tree;
				newRoot->right = new node();
				newRoot->right->rect = tree->rect;
				newRoot->right->rect.y += tree->rect.w;
				std::cout << "Make it longer! " << glm::to_string(newRoot->rect) << std::endl;
			}
			newNode = newRoot->right->insert(texId, size);
			tree = newRoot;
		}
		return newNode->rect;
	}

};
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

		std::cout << "atlas size: " << atlasSize.x << " " << atlasSize.y << std::endl;
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

			std::cout << "origin: " << origin[0] << " " << origin[1] << std::endl;
			std::cout << "size: " << size[0] << " " << size[1] << std::endl;
			devCtx.commandQueue.enqueueWriteImage(
					textureAtlas, true, origin, size, 0, 0, textureData[index++].data());
		}

		return textureAtlas;
	}

	unsigned clp2(unsigned x) {
	   x = x - 1;
	   x = x | (x >> 1);
	   x = x | (x >> 2);
	   x = x | (x >> 4);
	   x = x | (x >> 8);
	   x = x | (x >>16);
	   return x + 1;
	}


	glm::ivec2 computePositions() {
		std::vector<unsigned> ids;
		for(unsigned i = 0; i < textureData.size(); i++) {
			ids.push_back(i);
		}

		std::cout << "Number of IDs: " << ids.size() << std::endl;

		std::sort(ids.begin(), ids.end(), [this](unsigned a, unsigned b){ return area(textureData[a]) > area(textureData[b]); } );

		gli::storage* largestTexture = &textureData[ids[0]];

		float largestDim = glm::max(static_cast<float>(width(*largestTexture)), static_cast<float>(height(*largestTexture)));

		unsigned dim = static_cast<unsigned>(glm::pow(2.0f, glm::ceil(glm::log(largestDim)/glm::log(2.0f))));

		glm::ivec4 currentSize(0, 0, dim, dim);

		SpaceTree tree;
		for(auto i : ids) {
			std::cout << "ID: " << i << "-------------------------------------" << std::endl;
			rects[i] = tree.insert(i, glm::ivec2(width(textureData[i]), height(textureData[i])));
		}
		return glm::ivec2(clp2(tree.tree->rect.z), clp2(tree.tree->rect.w));
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
