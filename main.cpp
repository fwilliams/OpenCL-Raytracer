
#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "multi_pass_renderer.h"
#include "scenes/cornell_box.h"
#include "scenes/tiled_mirror_box.h"
#include "scenes/trippy_metal_box.h"


//#define RENDER_LIGHTS

using namespace std;

const int kWidth = 800;
const int kHeight = 600;
const int numReflectivePasses = 10;
const double maxViewDistance = 1000.0;
const bool kFullscreen = false;

GLuint renderTex;

void initOpenGL() {
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &renderTex);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glFinish();	// Texture needs to exist for openCL
}

template <typename Renderer>
void render(int delta, Renderer& rndr) {
	glm::mat4 viewMatrix;
	viewMatrix = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
	rndr.renderToTexture(renderTex, mat4ToFloat16(viewMatrix));

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
	glLoadIdentity();
	glFrustum(-0.5, 0.5, -0.5, 0.5, 0.5, 100.0);

	glPushAttrib(GL_CURRENT_BIT);

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(auto i = lights.begin(); i != lights.end(); i++) {
		GLfloat c[3] = {1.0, 1.0, 1.0};
		glColor3fv(c);
		glVertex3fv((GLfloat*)&i->position);
	}
	glEnd();

	glPopAttrib();

	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
#endif

	SDL_GL_SwapBuffers();
}

void update(int delta) {}

int main(int argc, char* argv[]) {
//	auto rndr = MultiPassRenderer<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(
//			TiledMirrorBox::buildTiledMirrorBox<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(glm::vec3(10.0, 10.0, 10.0)),
//			kWidth, kHeight, numReflectivePasses, maxViewDistance);

	auto rndr = MultiPassRenderer<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(
			CornellBox::buildCornellBox<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(glm::vec3(10.0, 10.0, 10.0)),
			kWidth, kHeight, numReflectivePasses, maxViewDistance);


//	auto rndr = MultiPassRenderer<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(
//			TrippyMetalBox::buildTrippyBoxScene<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(glm::vec2(10.0, 10.0), 10.0f),
//			kWidth, kHeight, numReflectivePasses, maxViewDistance);

	SDL_Init(SDL_INIT_EVERYTHING);

	Uint32 flags = SDL_OPENGL;
	if(kFullscreen) {
		flags |= SDL_FULLSCREEN;
		SDL_ShowCursor(0);
	}

	SDL_SetVideoMode(kWidth, kHeight, 32, flags);

	initOpenGL();

	bool loop = true;
	int lastTicks = SDL_GetTicks();
	bool first = true;
	while(loop) {
		int delta = SDL_GetTicks() - lastTicks;
		lastTicks = SDL_GetTicks();
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				loop = false;
			} else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				loop = false;
			}
			usleep(1000);
		}
		if(first) {
			render(1.0, rndr);
			first = false;
		}
		update(delta);
		usleep(30000);

		std::stringstream ss;
		ss << 1000.0f / delta;
		SDL_WM_SetCaption(ss.str().c_str(), 0);
	}

	return 0;
}
