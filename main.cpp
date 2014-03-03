#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <glm/glm.hpp>
#include "renderer.h"

const int kWidth = 800;
const int kHeight = 600;
const bool kFullscreen = false;

using namespace std;
using namespace glm;

GLuint renderTex;
mat4 viewMatrix;

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

renderer initOpenCL() {
	vector<Sphere> spheres;
	vector<Triangle> tris;
	vector<PointLight> lights;
	vector<Material> mats;

	// Spheres
	spheres.push_back(Sphere{1.0, cl_float3{{-4.0, 0.0, 5.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{4.0, 0.0, 5.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{0.0, -2.0, 5.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{0.0, 2.0, 5.0}}, 0});

	spheres.push_back(Sphere{1.0, cl_float3{{-4.0, 0.0, 10.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{4.0, 0.0, 10.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{0.0, -2.0, 10.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{0.0, 2.0, 10.0}}, 0});

	// Triangles
	tris.push_back(
			Triangle{
				cl_float3{{-3.0, -1.5, -6.0}},
				cl_float3{{ 3.0, -1.5, -6.0}},
				cl_float3{{ 0.0,  1.5, -6.0}},
				1});

	// Lights
	lights.push_back(
			PointLight{
				cl_float3{{-1.5, 0.1, 5.0}},
				cl_float3{{0.0, 1.0, 0.0}}});
	lights.push_back(
			PointLight{
				cl_float3{{1.5, 0.1, 5.0}},
				cl_float3{{1.0, 0.0, 0.0}}});
	lights.push_back(
			PointLight{
				cl_float3{{0.0, 0.1, 0.0}},
				cl_float3{{0.6, 0.6, 0.6}}});

	// Materials
	mats.push_back(
			Material{
				cl_float3{{0.8, 0.8, 0.8}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.5, 0.5, 0.5}}});

	// Materials
	mats.push_back(
			Material{
				cl_float3{{0.8, 0.8, 0.8}},
				cl_float3{{0.0, 0.0, 0.0}},
				cl_float3{{0.1, 0.1, 0.5}}});

	RenderParams params {100.0f, (cl_uint)tris.size(), (cl_uint)spheres.size(), (cl_uint)lights.size()};

	return renderer(spheres, tris, lights, mats, params, kWidth, kHeight);
}

void render(int delta, renderer& rndr) {
	rndr.renderToTexture(renderTex, nullptr);

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
	renderer rndr = initOpenCL();

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
