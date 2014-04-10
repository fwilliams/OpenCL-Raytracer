/*
 * texture_atlas_kd_tree.h
 *
 *  Created on: Apr 7, 2014
 *      Author: francis
 */

#ifndef TEXTURE_ATLAS_KD_TREE_H_
#define TEXTURE_ATLAS_KD_TREE_H_

struct TextureAtlasKDTree {
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

	node* root = nullptr;
	glm::ivec2 maxDims;
public:

	TextureAtlasKDTree(glm::ivec2 maxDims) : maxDims(maxDims) {}

	~TextureAtlasKDTree() {
		delete root;
	}

	glm::ivec2 getSize() {
		if(root != nullptr) {
			return glm::ivec2(root->rect.z, root->rect.w);
		} else {
			return glm::ivec2(0, 0);
		}
	}

	glm::ivec4 insert(unsigned texId, glm::ivec2 size) {
		// If the tree hasn't been initialized, initialize it with the maximum possible texture size
		if(root == nullptr) {
			root = new node();
			root->rect = glm::ivec4(0, 0, maxDims.x, maxDims.y);
		}

		// Try to insert the texture
		node* newNode = root->insert(texId, size);

		// If the texture doesn't fit, create a new tree containing the old one and an empty tree
		// the same size as the old one. Note that there's an invariant here that insertions will
		// be performed in sorted order from largest to smallest.
		// The resizing dimension alternates between left and right.
		if(newNode == nullptr) {
			node* newRoot = new node();
			if(root->rect.z <= root->rect.w) {
				newRoot->rect = root->rect;
				newRoot->rect.z *= 2;

				newRoot->left = root;
				newRoot->right = new node();
				newRoot->right->rect = root->rect;
				newRoot->right->rect.x += root->rect.z;
			} else {
				newRoot->rect = root->rect;
				newRoot->rect.w *= 2;

				newRoot->left = root;
				newRoot->right = new node();
				newRoot->right->rect = root->rect;
				newRoot->right->rect.y += root->rect.w;
			}

			// This is guaranteed to succeed because of the invariant that textures are inserted in
			// order of decreasing size, the first subtree was initialized with the largest possible dimensions.
			// Therefore an empty tree can contain any of the inserted textures
			newNode = newRoot->right->insert(texId, size);
			root = newRoot;
		}

		return newNode->rect;
	}
};



#endif /* TEXTURE_ATLAS_KD_TREE_H_ */
