/*
 * texture_array.h
 *
 *  Created on: Apr 7, 2014
 *      Author: francis
 */

#include <gli/gli.hpp>

#include <array>

#ifndef TEXTURE_ARRAY_H_
#define TEXTURE_ARRAY_H_

typedef cl_uint TextureHdl;
const cl_uint no_texture = 0;

struct TextureArray {
	typedef std::vector<gli::storage>::iterator iterator;

	TextureHdl createTexture(const gli::storage& strg) {
		storage.push_back(strg);
		return static_cast<TextureHdl>(storage.size());
	}

	template <size_t N>
	std::array<TextureHdl, N> createTextures(std::array<gli::storage, N>& strg) {
		std::array<TextureHdl, N> res;
		for(unsigned i = 0; i < strg.size(); i++) {
			res[i] = createTexture(strg[i]);
		}

		return res;
	}

	template <size_t N>
	std::array<TextureHdl, N> createTextures(std::initializer_list<gli::storage> strg) {
		std::array<TextureHdl, N> res;
		unsigned index = 0;
		for(auto i : strg) {
			res[index++] = createTexture(i);
		}

		return res;
	}

	gli::storage& operator[](TextureHdl hdl) {
		return storage[hdl - 1];
	}

	size_t size() const {
		return storage.size();
	}

	iterator begin() {
		return storage.begin();
	}

	iterator end() {
		return storage.end();
	}

	bool empty() const {
		return storage.empty();
	}

private:
	std::vector<gli::storage> storage;
};



#endif /* TEXTURE_ARRAY_H_ */
