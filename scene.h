/*
 * scene.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <CL/cl.hpp>
#include <memory>

#include "geometry_types.h"
#include "cl_device_context.h"
#ifndef SCENE_H_
#define SCENE_H_

template <typename T>
struct NullBuffer {
};

// Allow buffering on the CPU for data
template <template<class ...> class InputContainerType,
		  cl_device_type DEVICE_TYPE>
struct Scene {
	Scene(
		std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> deviceContext,
		InputContainerType<Sphere>& spheres,
		InputContainerType<Triangle>& tris,
		InputContainerType<PointLight>& pointLights,
		InputContainerType<Material>& materials) :
			clDeviceContext(deviceContext),
			numSpheres(spheres.size()),
			numTriangles(tris.size()),
			numPointLights(pointLights.size()),
			numMaterials(materials.size()) {
		packCLBuffers(spheres, tris, pointLights, materials);
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

	std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> getCLDeviceContext() const {
		return clDeviceContext;
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
	std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> clDeviceContext;
	cl::Buffer clTriangles, clSpheres, clPointlights, clMaterials;

	unsigned numSpheres;
	unsigned numTriangles;
	unsigned numPointLights;
	unsigned numMaterials;

	inline void packCLBuffers(
			InputContainerType<Sphere>& spheres,
			InputContainerType<Triangle>& tris,
			InputContainerType<PointLight>& pointLights,
			InputContainerType<Material>& materials) {

		size_t sphereByteSize = sizeof(Sphere) * spheres.size();
		clSpheres = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, sphereByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clSpheres, true, 0, sphereByteSize, spheres.data());

		size_t trisByteSize = sizeof(Triangle) * tris.size();
		clTriangles = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, trisByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clTriangles, true, 0, trisByteSize,tris.data());

		size_t lightByteSize = sizeof(PointLight) * pointLights.size();
		clPointlights = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, lightByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clPointlights, true, 0, lightByteSize, pointLights.data());

		size_t materialByteSize = sizeof(Material) * materials.size();
		clMaterials = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, materialByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clMaterials, true, 0, materialByteSize, materials.data());
	}
};



#endif /* SCENE_H_ */
