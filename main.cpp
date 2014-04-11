
#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer/renderer.h"
#include "scenes/tiled_mirror_box.h"
#include "orbit_camera.h"

const int kWidth = 512;
const int kHeight = 512;
const int numReflectivePasses = 2;
const double maxViewDistance = 1000000.0;

GLuint renderTex;

SDL_Cursor *arrow, *hand;
SDL_Window* displayWindow;
SDL_Renderer* displayRenderer;

OrbitCamera camera(glm::vec2(0.5), 4.0f, glm::vec2(1.0, 4.9));

void initOpenGL() {
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &renderTex);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glFinish();	// Texture needs to exist for openCL
}

template <typename Renderer>
void render(int delta, Renderer& rndr) {
	rndr.renderToTexture(renderTex, mat4ToFloat16(camera.getViewMatrix()));

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

	SDL_GL_SwapWindow(displayWindow);
}

void update(int delta, SDL_Event& evt) {
	camera.update(evt);
	if(camera.isMoving()) {
		SDL_SetCursor(hand);
	} else {
		SDL_SetCursor(arrow);
	}
}

int main(int argc, char* argv[]) {
	TextureArray textures;
	auto texIds = textures.createTextures<6>(
			{gli::load_dds("textures/tex1.dds"),
	  	  	 gli::load_dds("textures/tex2.dds"),
	  	  	 gli::load_dds("textures/tex3.dds"),
	  	  	 gli::load_dds("textures/bricks.dds"),
	  	  	 gli::load_dds("textures/bricks_normals.dds"),
	  	  	 gli::load_dds("textures/tex4.dds")});

	auto scene = TiledMirrorBox::buildTiledMirrorBox<BLINN_PHONG>(
			glm::vec3(10.0, 10.0, 10.0), glm::ivec2(0, 0),
			{{texIds[0], texIds[1], texIds[2],
			  texIds[3], texIds[4], texIds[5]}});

	auto rndr = Renderer<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(
			scene, textures, kWidth, kHeight,
			numReflectivePasses, maxViewDistance);

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(kWidth, kHeight, SDL_WINDOW_OPENGL, &displayWindow, &displayRenderer);

	arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);

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
			update(delta, e);
			usleep(1000);
		}
		if(true) {
			render(1.0, rndr);
			first = false;
		}

		std::stringstream ss;
		ss << 1000.0f / delta;
		SDL_SetWindowTitle(displayWindow, ss.str().c_str());
	}

	SDL_Quit();

	return 0;
}
