#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

const int kWidth = 800;
const int kHeight = 600;
const bool kFullscreen = false;

size_t global_work_size = kWidth * kHeight;

float viewMatrix[16];

float sphere1Pos[3] = { 0, 0, 10 };
float sphere2Pos[3] = { 0, 0, -10 };
float sphereVelocity = 1;
float sphereTransforms[2][16];

unsigned char* pixels;// = new unsigned char[kWidth * kHeight * 4];

cl_command_queue queue;
cl_kernel kernel;
cl_mem buffer, viewTransform, worldTransforms;

int InitOpenCL() {
	std::cout << "Initializing OpenCL" << std::endl;

	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_int error;

	clGetPlatformIDs(1, &platform, NULL);

	std::cout << "Got handle to OpenCL platform" << std::endl;


	// 2. Find a gpu device.
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device,
	NULL);

	std::cout << "Got handle to GPU device" << std::endl;


	// 3. Create a context and command queue on that device.
	context = clCreateContext( NULL, 1, &device,
	NULL, NULL, NULL);

	std::cout << "Created OpenCL context" << std::endl;


	queue = clCreateCommandQueue(context, device, 0, NULL);

	std::cout << "Created OpenCL command queue" << std::endl;


	// 4. Perform runtime source compilation, and obtain kernel entry point.
	std::ifstream file("kernel.cl");
	std::string source;
	while (!file.eof()) {
		char line[256];
		file.getline(line, 255);
		source += line;
		source += '\n';
	}

	cl_ulong maxSize;
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong),
			&maxSize, 0);

	const char* str = source.c_str();
	size_t len = source.size();
	cl_program program = clCreateProgramWithSource(context, 1, &str, (const size_t*)&len, NULL);
	cl_int result = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	if (result) {
		std::cout << "Error during compilation! (" << result << ")" << std::endl;
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char *log = (char *) malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		std::cout << log << std::endl;

		return 1;
	}

	kernel = clCreateKernel(program, "main", NULL);

	std::cout << "Created ray tracer kernel" << std::endl;


	// 5. Create a data buffer.
	buffer = clCreateBuffer(
			context, CL_MEM_WRITE_ONLY,
			kWidth * kHeight * sizeof(cl_float4), NULL, 0);
	viewTransform = clCreateBuffer(
			context, CL_MEM_READ_WRITE,
			16 * sizeof(cl_float), NULL, 0);

	worldTransforms = clCreateBuffer(
			context, CL_MEM_READ_WRITE,
			16 * sizeof(cl_float) * 2, NULL, 0);

	clSetKernelArg(kernel, 0, sizeof(buffer), (void*) &buffer);
	clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*) &kWidth);
	clSetKernelArg(kernel, 2, sizeof(cl_uint), (void*) &kWidth);
	clSetKernelArg(kernel, 3, sizeof(viewTransform), (void*) &viewTransform);
	clSetKernelArg(kernel, 4, sizeof(worldTransforms), (void*) &worldTransforms);

	std::cout << "Created input/output data buffers" << std::endl;

	return 0;
}

void Render(int delta) {
	clEnqueueNDRangeKernel(queue, kernel, 1,NULL, &global_work_size, NULL, 0, NULL, NULL);

	// 7. Look at the results via synchronous buffer map.
	cl_float4 *ptr = (cl_float4 *)
		clEnqueueMapBuffer(
			queue, buffer, CL_TRUE, CL_MAP_READ, 0,
			kWidth * kHeight * sizeof(cl_float4), 0, NULL, NULL, NULL);

	cl_float *viewTransformPtr = (cl_float *)
		clEnqueueMapBuffer(
			queue, viewTransform, CL_TRUE,
			CL_MAP_WRITE, 0, 16 * sizeof(cl_float), 0, NULL, NULL, NULL);

	cl_float *worldTransformsPtr = (cl_float *) clEnqueueMapBuffer(queue,
			worldTransforms,
			CL_TRUE,
			CL_MAP_WRITE, 0, 16 * sizeof(cl_float) * 2, 0, NULL, NULL, NULL);

	memcpy(viewTransformPtr, viewMatrix, sizeof(float) * 16);
	memcpy(worldTransformsPtr, sphereTransforms[0], sizeof(float) * 16);
	memcpy(worldTransformsPtr + 16, sphereTransforms[1], sizeof(float) * 16);

	clEnqueueUnmapMemObject(queue, viewTransform, viewTransformPtr, 0, 0, 0);
	clEnqueueUnmapMemObject(queue, worldTransforms, worldTransformsPtr, 0, 0,
			0);

	for (int i = 0; i < kWidth * kHeight; i++) {
		pixels[i * 4] = ptr[i].s[0] * 255;
		pixels[i * 4 + 1] = ptr[i].s[1] * 255;
		pixels[i * 4 + 2] = ptr[i].s[2] * 255;
		pixels[i * 4 + 3] = 1;
	}

	glBindTexture(GL_TEXTURE_2D, 1);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, kWidth, kHeight, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);


	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, 1, -1, 1, 100);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex3f(-1, -1, -1);
	glTexCoord2f(0, 0);
	glVertex3f(-1, 1, -1);
	glTexCoord2f(1, 0);
	glVertex3f(1, 1, -1);
	glTexCoord2f(1, 1);
	glVertex3f(1, -1, -1);
	glEnd();

	SDL_GL_SwapBuffers();
	clFinish(queue);
}

void Update(int delta) {
	int count;
	Uint8* keys = SDL_GetKeyState(&count);

	float translate[3] = { 0, 0, 0 };
	if (keys[SDLK_DOWN]) {
		translate[2] = -0.01 * delta;
	}
	if (keys[SDLK_UP]) {
		translate[2] = 0.01 * delta;	}
	if (keys[SDLK_LEFT]) {
		translate[0] = -0.01 * delta;
	}
	if (keys[SDLK_RIGHT]) {
		translate[0] = 0.01 * delta;
	}

	int x, y;
	SDL_GetMouseState(&x, &y);
	int relX = (kWidth / 2.0f - x) * delta;
	int relY = (kHeight / 2.0f - y) * delta;
	SDL_WarpMouse(kWidth / 2.0f, kHeight / 2.0f);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glMultMatrixf(viewMatrix);
	glTranslatef(translate[0], translate[1], translate[2]);

	if (relX != 0) {
		glRotatef(-relX / 200.0f, 0, 1, 0);
	}
	if (relY != 0) {
		glRotatef(-relY / 200.0f, 1, 0, 0);
	}

	glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);

	// Sphere Transforms
	glLoadIdentity();
	glTranslatef(0, 0, sphere1Pos[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, sphereTransforms[0]);

	glLoadIdentity();
	glTranslatef(0, 0, sphere2Pos[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, sphereTransforms[1]);

	sphere1Pos[2] += sphereVelocity * delta / 30.0f;
	sphere2Pos[2] += sphereVelocity * (-1) * delta / 30.0f;

	if (sphere1Pos[2] > 50) {
		sphereVelocity = -1;
	} else if (sphere1Pos[2] < -50) {
		sphereVelocity = 1;
	}
}

int main(int argc, char* argv[]) {
	if(InitOpenCL()) {
		return 1;
	}

	pixels = new unsigned char[kWidth * kHeight * 4];

	memset(viewMatrix, 0, sizeof(float) * 16);
	viewMatrix[0] = viewMatrix[5] = viewMatrix[10] = viewMatrix[15] = 1;

	SDL_Init(SDL_INIT_EVERYTHING);

	Uint32 flags = SDL_OPENGL;
	if (kFullscreen) {
		flags |= SDL_FULLSCREEN;

		SDL_ShowCursor(0);
	}

	SDL_SetVideoMode(kWidth, kHeight, 32, flags);

	glEnable(GL_TEXTURE_2D);

	bool loop = true;
	int lastTicks = SDL_GetTicks();
	while (loop) {
		int delta = SDL_GetTicks() - lastTicks;
		lastTicks = SDL_GetTicks();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				loop = false;
			} else if (e.type == SDL_KEYDOWN
					&& e.key.keysym.sym == SDLK_ESCAPE) {
				loop = false;
			}
		}

		Update(delta);
		Render(delta);

		std::stringstream ss;
		ss << 1000.0f / delta;
		SDL_WM_SetCaption(ss.str().c_str(), 0);
	}

	delete[] pixels;

	return 0;
}
