/*
 * texture_atlas_kd_tree.h
 *
 *  Created on: Apr 7, 2014
 *      Author: francis
 */

#include <glm/glm.hpp>

#ifndef TEXTURE_ATLAS_KD_TREE_H_
#define TEXTURE_ATLAS_KD_TREE_H_

struct TextureAtlasKDTree {
private:
	struct node {
		static const unsigned NO_TEXTURE = -2;

		node() : rect(glm::ivec4(0)), left(nullptr), right(nullptr), texId(NO_TEXTURE) {}
		~node();

		node* insert(unsigned tex, const glm::ivec2& size);

		glm::ivec4 rect;
		node* left;
		node* right;
		int texId;
	};

	node* root = nullptr;
	glm::ivec2 maxDims;
public:

	TextureAtlasKDTree(glm::ivec2 maxDims) : maxDims(maxDims) {}

	~TextureAtlasKDTree();

	glm::ivec2 getSize();

	glm::ivec4 insert(unsigned texId, glm::ivec2 size);
};



#endif /* TEXTURE_ATLAS_KD_TREE_H_ */
