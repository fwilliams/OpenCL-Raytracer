/*
 * cl_device_context.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <CL/cl.hpp>

#include <vector>

#ifndef CL_DEVICE_CONTEXT_H_
#define CL_DEVICE_CONTEXT_H_

template <cl_device_type DEVICE_TYPE>
struct ClDeviceContext {
	ClDeviceContext() {
		std::vector<cl::Platform> allPlatforms;
		cl::Platform::get(&allPlatforms);

		std::vector<cl::Device> allDevices;
		allPlatforms[0].getDevices(DEVICE_TYPE, &allDevices);

		device = allDevices[0];

		context = cl::Context( { device });

		commandQueue = cl::CommandQueue(context, allDevices[0]);
	}

	cl::Context context;
	cl::Device device;
	cl::CommandQueue commandQueue;
};

#endif /* CL_DEVICE_CONTEXT_H_ */
