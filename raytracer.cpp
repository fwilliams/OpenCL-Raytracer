#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "renderer.h"

const int kWidth = 800;
const int kHeight = 600;
const bool kFullscreen = false;

using namespace std;
using namespace glm;

renderer* rndr;

int InitOpenCL() {
	vector<Sphere> spheres;
	vector<Triangle> tris;
	vector<PointLight> lights;
	vector<Material> mats;

	spheres.push_back(Sphere{1.0, vec3(5.0, 0.0, 5.0), 0});
	tris.push_back(Triangle{{vec3(-0.5, 0.0, 0.0), vec3(0.5, 0.0, 0.0), vec3(0.0, 1.0, 0.0)}, 0});
	lights.push_back(PointLight{vec3(0.0, 7.0, 0.0), vec3(0.5, 0.5, 0.5)});
	mats.push_back(Material{vec3(0.1, 0.1, 0.1), vec3(0.0), vec3(0.9, 0.9, 0.9)});

	rndr = new renderer(spheres, tris, lights, mats);

	return 0;
}

void Render(int delta) {
	rndr->renderToTexture();
	glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0+1);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
	glTranslatef(translate[0], translate[1], translate[2]);

	if (relX != 0) {
		glRotatef(-relX / 200.0f, 0, 1, 0);
	}
	if (relY != 0) {
		glRotatef(-relY / 200.0f, 1, 0, 0);
	}
}

int main(int argc, char* argv[]) {
	if(InitOpenCL()) {
		return 1;
	}

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

	return 0;
}
