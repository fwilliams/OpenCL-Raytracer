/*
 * box.h
 *
 *  Created on: Apr 1, 2014
 *      Author: francis
 */

#include "data_types.h"
#include <array>
#include <vector>

#ifndef BOX_H_
#define BOX_H_

struct Box {
	enum Face {TOP, BOTTOM, FRONT, BACK, RIGHT, LEFT};

	static void makeBoxInterior(std::vector<Triangle>& tris, const glm::vec3& wallSize, const unsigned matBase = 0) {
		makeBox<0>(tris, wallSize,
				interiorNormals(),
				offsettedIndices<6>(matBase),
				{{}},
				glm::ivec3(1));
	}

	static void makeBoxExterior(std::vector<Triangle>& tris, const glm::vec3& wallSize, const unsigned matBase = 0) {
		makeBox<0>(tris, wallSize,
				exteriorNormals(),
				offsettedIndices<6>(matBase),
				{{}},
				glm::ivec3(1));
	}

	template <unsigned N>
	static void makePartialBoxExterior(
			std::vector<Triangle>& tris, const glm::vec3& wallSize,
			const std::array<Face, N>& disabledFaces, const unsigned matBase = 0) {

		std::array<cl_float3, 6 - N> normals;
		for(unsigned i = 0, j = 0; i < 6; i++) {
			if(!isWallDisabled<N>(disabledFaces, static_cast<Face>(i))) {
				normals[j] = exteriorNormals()[i];
				j += 1;
			}
		}

		makeBox<N>(tris, wallSize, normals,
				offsettedIndices<6-N>(matBase), disabledFaces, glm::ivec3(1));
	}

	template <unsigned N>
	static void makePartialBoxInterior(
			std::vector<Triangle>& tris, const glm::vec3& wallSize,
			const std::array<Face, N>& disabledFaces, const unsigned matBase = 0) {

		std::array<cl_float3, 6 - N> normals;
		for(unsigned i = 0, j = 0; i < 6; i++) {
			if(!isWallDisabled<N>(disabledFaces, static_cast<Face>(i))) {
				normals[j] = interiorNormals()[i];
				j += 1;
			}
		}

		makeBox<N>(tris, wallSize, normals,
				offsettedIndices<6-N>(matBase), disabledFaces, glm::ivec3(1));
	}

	template <unsigned N>
	static void makePartialBoxInterior(
			std::vector<Triangle>& tris, const glm::vec3& wallSize,
			const std::array<Face, N>& disabledFaces, const std::array<cl_uint, 6-N>& matInds) {

		std::array<cl_float3, 6 - N> normals;
		for(unsigned i = 0, j = 0; i < 6; i++) {
			if(!isWallDisabled<N>(disabledFaces, static_cast<Face>(i))) {
				normals[j] = interiorNormals()[i];
				j += 1;
			}
		}

		makeBox<N>(tris, wallSize, normals, matInds, disabledFaces, glm::ivec3(1));
	}

private:

	static constexpr std::array<cl_float3, 6> interiorNormals() {
		return std::array<cl_float3, 6>{{
			cl_float3{{0.0, -1.0, 0.0}},
			cl_float3{{0.0, 1.0, 0.0}},
			cl_float3{{0.0, 0.0, 1.0}},
			cl_float3{{0.0, 0.0, -1.0}},
			cl_float3{{-1.0, 0.0, 0.0}},
			cl_float3{{1.0, 0.0, 0.0}}
		}};
	}

	static constexpr std::array<cl_float3, 6> exteriorNormals() {
		return std::array<cl_float3, 6>{{
			cl_float3{{0.0, 1.0, 0.0}},
			cl_float3{{0.0, -1.0, 0.0}},
			cl_float3{{0.0, 0.0, -1.0}},
			cl_float3{{0.0, 0.0, 1.0}},
			cl_float3{{1.0, 0.0, 0.0}},
			cl_float3{{-1.0, 0.0, 0.0}}
		}};
	}

	template <unsigned N>
	static bool isWallDisabled(const std::array<Face, N>& arr, Face wall) {
		for(unsigned i = 0; i < N; i++) {
			if(arr[i] == wall) {
				return true;
			}
		}
		return false;
	}

	template <unsigned N>
	static std::array<cl_uint, N> offsettedIndices(unsigned base) {
		std::array<cl_uint, N> indices;

		for(unsigned i = 0; i < N; i++) {
			indices[i] = base + i;
		}

		return indices;
	}

	template <unsigned N>
	static inline void makeBox(
				std::vector<Triangle>& tris, const glm::vec3& wallSize,
				const std::array<cl_float3, 6-N>& normals,
				const std::array<cl_uint, 6-N>& materialIndices,
				const std::array<Face, N>& disabledFaces,
				const glm::ivec3& numTris) {
		const glm::vec3 halfSize = wallSize/2.0f;
		const glm::vec3 d = wallSize / static_cast<glm::vec3>(numTris);
		unsigned index = 0;
		for(unsigned i = 0; i < numTris.x; i++) {
			for(unsigned j = 0; j < numTris.y; j++) {
				for(unsigned k = 0; k < numTris.z; k++) {
					// Top plane
					if(!isWallDisabled<N>(disabledFaces, Face::TOP)) {
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     halfSize.y, -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, halfSize.y, -halfSize.z+(k+1)*d.z}},
								cl_float3{{-halfSize.x+i*d.x,     halfSize.y, -halfSize.z+(k+1)*d.z}},
								normals[index],
								materialIndices[index]});
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     halfSize.y, -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, halfSize.y, -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, halfSize.y, -halfSize.z+(k+1)*d.z}},
								normals[index],
								materialIndices[index]});
						index = (index + 1) % (6-N);
					}

					// Bottom plane
					if(!isWallDisabled<N>(disabledFaces, Face::BOTTOM)) {
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y, -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y, -halfSize.z+(k+1)*d.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y, -halfSize.z+(k+1)*d.z}},
								normals[index],
								materialIndices[index]});
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y, -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y, -halfSize.z+(k+1)*d.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y, -halfSize.z+k*d.z}},
								normals[index],
								materialIndices[index]});
						index = (index + 1) % (6-N);
					}

					// Front Plane
					if(!isWallDisabled<N>(disabledFaces, Face::FRONT)) {
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y+j*d.y, 	 -halfSize.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y+(j+1)*d.y, -halfSize.z}},
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y+(j+1)*d.y, -halfSize.z}},
								normals[index],
								materialIndices[index]});
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y+j*d.y,    -halfSize.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y+j*d.y,    -halfSize.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y+(j+1)*d.y,-halfSize.z}},
								normals[index],
								materialIndices[index]});
						index = (index + 1) % (6-N);
					}

					// Back Plane
					if(!isWallDisabled<N>(disabledFaces, Face::BACK)) {
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y+j*d.y, 	 halfSize.z}},
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y+(j+1)*d.y, halfSize.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y+(j+1)*d.y, halfSize.z}},
								normals[index],
								materialIndices[index]});
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x+i*d.x,     -halfSize.y+j*d.y,     halfSize.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y+(j+1)*d.y, halfSize.z}},
								cl_float3{{-halfSize.x+(i+1)*d.x, -halfSize.y+j*d.y,     halfSize.z}},
								normals[index],
								materialIndices[index]});
						index = (index + 1) % (6-N);
					}

					// Right Plane
					if(!isWallDisabled<N>(disabledFaces, Face::RIGHT)) {
						tris.push_back(
							Triangle{
								cl_float3{{halfSize.x, -halfSize.y+j*d.y,     -halfSize.z+k*d.z}},
								cl_float3{{halfSize.x, -halfSize.y+j*d.y,     -halfSize.z+(k+1)*d.z}},
								cl_float3{{halfSize.x, -halfSize.y+(j+1)*d.y, -halfSize.z+(k+1)*d.z}},
								normals[index],
								materialIndices[index]});
						tris.push_back(
							Triangle{
								cl_float3{{halfSize.x, -halfSize.y+j*d.y,     -halfSize.z+k*d.z}},
								cl_float3{{halfSize.x, -halfSize.y+(j+1)*d.y, -halfSize.z+(k+1)*d.z}},
								cl_float3{{halfSize.x, -halfSize.y+(j+1)*d.y, -halfSize.z+k*d.z}},
								normals[index],
								materialIndices[index]});
						index = (index + 1) % (6-N);
					}

					// Left Plane
					if(!isWallDisabled<N>(disabledFaces, Face::LEFT)) {
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x, -halfSize.y+j*d.y,     -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x, -halfSize.y+(j+1)*d.y, -halfSize.z+(k+1)*d.z}},
								cl_float3{{-halfSize.x, -halfSize.y+j*d.y,     -halfSize.z+(k+1)*d.z}},
								normals[index],
								materialIndices[index]});
						tris.push_back(
							Triangle{
								cl_float3{{-halfSize.x, -halfSize.y+j*d.y,     -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x, -halfSize.y+(j+1)*d.y, -halfSize.z+k*d.z}},
								cl_float3{{-halfSize.x, -halfSize.y+(j+1)*d.y, -halfSize.z+(k+1)*d.z}},
								normals[index],
								materialIndices[index]});
						index = (index + 1) % (6-N);
					}
				}
			}
		}
	}
};



#endif /* BOX_H_ */
