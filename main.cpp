#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"
#include "scene.h"

#define RENDER_LIGHTS

using namespace std;

const int kWidth = 512;
const int kHeight = 512;
const bool kFullscreen = false;

GLuint renderTex;

vector<Sphere> spheres;
vector<Triangle> tris;
vector<PointLight> lights;
vector<Material> mats;

void initOpenGL() {
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &renderTex);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, 1, -1, 1, 1);
	glMatrixMode(GL_MODELVIEW);

	glFinish();	// Texture needs to exist for openCL
}

renderer<vector, CL_DEVICE_TYPE_GPU> initOpenCL() {
	// Spheres
	spheres.push_back(Sphere{1.0, cl_float3{{-2.5, -0.0, -3.0}}, 1});
	spheres.push_back(Sphere{1.0, cl_float3{{ 2.5, -0.0, -3.0}}, 0});

	spheres.push_back(Sphere{1.0, cl_float3{{-2.5, -0.0, -5.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{ 2.5, -0.0, -5.0}}, 1});


	float planeWidth = 10.0;
	float planeHeight = 10.0;
	float planeSeparation = 2.4;
	unsigned trisX = 1;
	unsigned trisY = 1;

	float halfSeperation = planeSeparation/2.0;
	float halfWidth = planeWidth/2.0;
	float halfHeight = planeHeight/2.0;
	float dX = planeWidth/trisX;
	float dY = planeWidth/trisY;

	for(unsigned i = 0; i < trisX; i++) {
		for(unsigned j = 0; j < trisY; j++) {
			tris.push_back(
				Triangle{
					cl_float3{{-halfWidth+i*dX,     halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, halfSeperation, -halfHeight+(j+1)*dY}},
					cl_float3{{-halfWidth+i*dX,     halfSeperation, -halfHeight+(j+1)*dY}},
					3});
			tris.push_back(
				Triangle{
					cl_float3{{-halfWidth+i*dX,     halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, halfSeperation, -halfHeight+(j+1)*dY}},
					3});

			tris.push_back(
				Triangle{
					cl_float3{{-halfWidth+i*dX,     -halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, -halfSeperation, -halfHeight+(j+1)*dY}},
					cl_float3{{-halfWidth+i*dX,     -halfSeperation, -halfHeight+(j+1)*dY}},
					3});
			tris.push_back(
				Triangle{
					cl_float3{{-halfWidth+i*dX,     -halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, -halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, -halfSeperation, -halfHeight+(j+1)*dY}},
					3});
		}
	}

	// Lights
	lights.push_back(
			PointLight{
				cl_float3{{-1.0, 0.1, -3.0}},
				cl_float3{{0.7, 0.7, 0.7}}});
	lights.push_back(
			PointLight{
				cl_float3{{1.0, 0.1, -3.0}},
				cl_float3{{0.7, 0.7, 0.7}}});
	lights.push_back(
			PointLight{
				cl_float3{{0.0, 0.1, 1.0}},
				cl_float3{{0.6, 0.6, 0.6}}});

	// Materials
	mats.push_back(
			Material{
				cl_float3{{0.2, 0.2, 0.2}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.1, 0.1, 0.6}}});

	mats.push_back(
			Material{
				cl_float3{{0.2, 0.2, 0.2}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.6, 0.1, 0.1}}});

	mats.push_back(
			Material{
				cl_float3{{0.99, 0.99, 0.99}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.3, 0.3, 0.3}}});

	mats.push_back(
			Material{
				cl_float3{{0.99, 0.99, 0.99}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.3, 0.3, 0.3}}});

	auto devCtx = std::make_shared<ClDeviceContext<CL_DEVICE_TYPE_GPU>>();
	auto scene = std::make_shared<Scene<std::vector, CL_DEVICE_TYPE_GPU>>(
			devCtx, spheres, tris, lights, mats);
	return renderer<vector, CL_DEVICE_TYPE_GPU>(scene, kWidth, kHeight);
}

void render(int delta, renderer<vector, CL_DEVICE_TYPE_GPU>& rndr) {
	glm::mat4 viewMatrix;
	viewMatrix = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
	rndr.renderToTexture(renderTex, glm::value_ptr(viewMatrix));

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, renderTex);
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
	glBindTexture(GL_TEXTURE_2D, 0);


#ifdef RENDER_LIGHTS
	glMatrixMode(GL_PROJECTION);
	glFrustum(-0.5, 0.5, -0.5, 0.5, 0.5, 100.0);

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(auto i = lights.begin(); i != lights.end(); i++) {
		glColor3fv((GLfloat*)&i->power);
		glVertex3fv((GLfloat*)&i->position);
	}
	glEnd();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
#endif

	SDL_GL_SwapBuffers();
}

void update(int delta) {
//	int count;
//	Uint8* keys = SDL_GetKeyState(&count);
//
//	float translate[3] = { 0, 0, 0 };
//	if (keys[SDLK_DOWN]) {
//		translate[2] = -0.01 * delta;
//	}
//	if (keys[SDLK_UP]) {
//		translate[2] = 0.01 * delta;	}
//	if (keys[SDLK_LEFT]) {
//		translate[0] = -0.01 * delta;
//	}
//	if (keys[SDLK_RIGHT]) {
//		translate[0] = 0.01 * delta;
//	}
//
//	int x, y;
//	SDL_GetMouseState(&x, &y);
//	int relX = (kWidth / 2.0f - x) * delta;
//	int relY = (kHeight / 2.0f - y) * delta;
//	SDL_WarpMouse(kWidth / 2.0f, kHeight / 2.0f);
//
//	glMatrixMode(GL_MODELVIEW);
//
//	glLoadIdentity();
//	glTranslatef(translate[0], translate[1], translate[2]);
//
//	if (relX != 0) {
//		glRotatef(-relX / 200.0f, 0, 1, 0);
//	}
//	if (relY != 0) {
//		glRotatef(-relY / 200.0f, 1, 0, 0);
//	}
}

int main(int argc, char* argv[]) {
	renderer<vector, CL_DEVICE_TYPE_GPU> rndr = initOpenCL();

	SDL_Init(SDL_INIT_EVERYTHING);

	Uint32 flags = SDL_OPENGL;
	if (kFullscreen) {
		flags |= SDL_FULLSCREEN;

		SDL_ShowCursor(0);
	}

	SDL_SetVideoMode(kWidth, kHeight, 32, flags);

	initOpenGL();

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

		update(delta);
		render(delta, rndr);

		std::stringstream ss;
		ss << 1000.0f / delta;
		SDL_WM_SetCaption(ss.str().c_str(), 0);
	}

	return 0;
}
