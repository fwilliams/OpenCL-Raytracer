/*
 * cl_device_context.h
 *
 *  Created on: Mar 11, 2014
 *      Author: francis
 */

#include <CL/cl.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

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

	cl::Program createProgramFromFile(std::string& filename, std::map<std::string, std::string>& defines ) {
		std::string programString;
		std::ifstream file;
		file.exceptions(std::ifstream::badbit);
		file.open(filename.c_str());

		while (!file.eof()) {
			std::string str;
			std::getline(file, str);
			programString += str;
			programString.push_back('\n');
		}

		file.close();

		cl::Program::Sources sources;
		sources.push_back({ programString.c_str(), programString.size() });

		std::string defineString;
		for(auto i = defines.begin(); i != defines.end(); i++) {
			defineString.append(
					std::string("-D")+i->first+std::string("=")+i->second+std::string(" "));
		}

		cl::Program program(context, sources);

		try {
			program.build((std::vector<cl::Device>){ device }, defineString.c_str());
		} catch (cl::Error& e) {
			std::cerr << std::string(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)) << std::endl;
		}

		return program;
	}
};

#endif /* CL_DEVICE_CONTEXT_H_ */