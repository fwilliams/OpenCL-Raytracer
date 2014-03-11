/*
 * scene.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <CL/cl.hpp>
#include <algorithm>

#include "geometry_types.h"

#ifndef SCENE_H_
#define SCENE_H_

template <typename T>
struct NullBuffer {
};

// Allow buffering on the CPU for data
template <template<class ...> class InputContainerType, template<class> class CPUBuffer = NullBuffer>
struct Scene {
	Scene(
		cl::Context& ctx,
		cl::CommandQueue& queue,
		InputContainerType<Sphere>& spheres,
		InputContainerType<Triangle>& tris,
		InputContainerType<PointLight>& pointLights,
		InputContainerType<Material>& materials) :
			  numSpheres(spheres.size()),
			  numTriangles(tris.size()),
			  numPointLights(pointLights.size()),
			  numMaterials(materials.size()) {
		packCLBuffers(ctx, queue, spheres, tris, pointLights, materials);
	}

	const cl::Buffer& getSphereBuffer() const {
		return clSpheres;
	}

	const cl::Buffer& getTriangleBuffer() const {
		return clTriangles;
	}

	const cl::Buffer& getPointLightBuffer() const {
		return clPointlights;
	}

	const cl::Buffer& getMaterialBuffer() const {
		return clMaterials;
	}

	unsigned getNumSpheres() const {
		return numSpheres;
	}

	unsigned getNumTriangles() const {
		return numTriangles;
	}

	unsigned getNumPointLights() const {
		return numPointLights;
	}

	unsigned getNumMaterials() const {
		return numMaterials;
	}

private:
	cl::Buffer clTriangles, clSpheres, clPointlights, clMaterials;
//	CPUBuffer<Triangle> cpuTriangles;
//	CPUBuffer<Sphere> cpuSpheres;
//	CPUBuffer<PointLight> cpuPointLights;
//	CPUBuffer<Material> cpuMaterials;

	unsigned numSpheres;
	unsigned numTriangles;
	unsigned numPointLights;
	unsigned numMaterials;

	inline void packCLBuffers(
			cl::Context& ctx,
			cl::CommandQueue& cmdQueue,
			InputContainerType<Sphere>& spheres,
			InputContainerType<Triangle>& tris,
			InputContainerType<PointLight>& pointLights,
			InputContainerType<Material>& materials) {

		size_t sphereByteSize = sizeof(Sphere) * spheres.size();
		clSpheres = cl::Buffer(ctx, CL_MEM_READ_ONLY, sphereByteSize);
		cmdQueue.enqueueWriteBuffer(spheres, true, 0, sphereByteSize, spheres.data());

		size_t trisByteSize = sizeof(Triangle) * tris.size();
		clTriangles = cl::Buffer(ctx, CL_MEM_READ_ONLY, trisByteSize);
		cmdQueue.enqueueWriteBuffer(tris, true, 0, trisByteSize,tris.data());

		size_t lightByteSize = sizeof(PointLight) * pointLights.size();
		clPointlights = cl::Buffer(ctx, CL_MEM_READ_ONLY, lightByteSize);
		cmdQueue.enqueueWriteBuffer(pointLights, true, 0, lightByteSize, pointLights.data());

		size_t materialByteSize = sizeof(Material) * materials.size();
		clMaterials = cl::Buffer(ctx, CL_MEM_READ_ONLY, materialByteSize);
		cmdQueue.enqueueWriteBuffer(materials, true, 0, materialByteSize, materials.data());
	}

//	inline void packCPUBuffers(
//			InputContainerType<Sphere>& spheres,
//			InputContainerType<Triangle>& tris,
//			InputContainerType<PointLight>& pointLights,
//			InputContainerType<Material>& materials) {
//		std::copy(spheres.begin(), spheres.end(), cpuSpheres.begin());
//		std::copy(tris.begin(), tris.end(), cpuTriangles.begin());
//		std::copy(pointLights.begin(), pointLights.end(), cpuPointLights.begin());
//		std::copy(materials.begin(), materials.end(), cpuMaterials.begin());
//	}
};



#endif /* SCENE_H_ */
