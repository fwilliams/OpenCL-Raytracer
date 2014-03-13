/*
 * scene.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <CL/cl.hpp>
#include <memory>
#include <algorithm>
#include <iterator>

#include "geometry_types.h"
#include "cl_device_context.h"

#ifndef SCENE_H_
#define SCENE_H_

template <template<class ...> class InputContainerType,
		  cl_device_type DEVICE_TYPE,
		  template<class ...> class StorageContainerType = std::vector>
struct Scene {
	template <typename SphereIter, typename TriIter, typename LightIter, typename MatIter>
	Scene(std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> deviceContext,
		  SphereIter sphereBegin, SphereIter sphereEnd,
		  TriIter triBegin, TriIter triEnd,
		  LightIter lightBegin, LightIter lightEnd,
		  MatIter matBegin, MatIter matEnd) : clDeviceContext(deviceContext) {

		cpuSpheres.resize(std::distance(sphereBegin, sphereEnd));
		cpuTriangles.resize(std::distance(triBegin, triEnd));
		cpuPointLights.resize(std::distance(lightBegin, lightEnd));
		cpuMaterials.resize(std::distance(matBegin, matEnd));

		std::copy(sphereBegin, sphereEnd, cpuSpheres.begin());
		std::copy(triBegin, triEnd, cpuTriangles.begin());
		std::copy(lightBegin, lightEnd, cpuPointLights.begin());
		std::copy(matBegin, matEnd, cpuMaterials.begin());

		packCLBuffers2();
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
		return cpuSpheres.size();
	}

	unsigned getNumTriangles() const {
		return cpuTriangles.size();
	}

	unsigned getNumPointLights() const {
		return cpuPointLights.size();
	}

	unsigned getNumMaterials() const {
		return cpuMaterials.size();
	}

private:
	StorageContainerType<Triangle> cpuTriangles;
	StorageContainerType<Sphere> cpuSpheres;
	StorageContainerType<PointLight> cpuPointLights;
	StorageContainerType<Material> cpuMaterials;

	std::shared_ptr<ClDeviceContext<DEVICE_TYPE>> clDeviceContext;
	cl::Buffer clTriangles, clSpheres, clPointlights, clMaterials;

	inline void packCLBuffers2() {
		size_t sphereByteSize = sizeof(Sphere) * cpuSpheres.size();
		clSpheres = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, sphereByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clSpheres, true, 0, sphereByteSize, cpuSpheres.data());

		size_t trisByteSize = sizeof(Triangle) * cpuTriangles.size();
		clTriangles = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, trisByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clTriangles, true, 0, trisByteSize, cpuTriangles.data());

		size_t lightByteSize = sizeof(PointLight) * cpuPointLights.size();
		clPointlights = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, lightByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clPointlights, true, 0, lightByteSize, cpuPointLights.data());

		size_t materialByteSize = sizeof(Material) * cpuMaterials.size();
		clMaterials = cl::Buffer(clDeviceContext->context, CL_MEM_READ_ONLY, materialByteSize);
		clDeviceContext->commandQueue.enqueueWriteBuffer(clMaterials, true, 0, materialByteSize, cpuMaterials.data());
	}
};



#endif /* SCENE_H_ */
