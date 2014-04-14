#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

#include "renderer.h"
#include "scenes/tiled_mirror_box.h"
#include "orbit_camera.h"
#include "sdl_app.h"

#include "scenes/kaleidescope.h"


struct App {
	static const int width = 512;
	static const int height = 512;

	const int numReflectivePasses = 0;
	const double maxViewDistance = 10000.0;

	GLuint renderTex;
	SDL_Cursor *arrow, *hand;
	OrbitCamera camera = OrbitCamera(glm::vec3(0.005, 0.005, 0.5), 4.0f, glm::vec2(1.0, 4.9));
	Renderer<CL_DEVICE_TYPE_GPU, BLINN_PHONG>* rndr;


	~App() {
		delete rndr;
	}

	void initGL() {
		glEnable(GL_TEXTURE_2D);

		glGenTextures(1, &renderTex);
		glBindTexture(GL_TEXTURE_2D, renderTex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		glFinish();	// Texture needs to exist for openCL
	}


	void init() {
		Scene<BLINN_PHONG> scene;

		auto texIds = scene.textures.createTextures<6>(
				{gli::load_dds("textures/tex1.dds"),
				 gli::load_dds("textures/tex2.dds"),
				 gli::load_dds("textures/tex3.dds"),
				 gli::load_dds("textures/bricks.dds"),
				 gli::load_dds("textures/bricks_normals.dds"),
				 gli::load_dds("textures/tex4.dds")});

		TiledMirrorBox::buildTiledMirrorBox<BLINN_PHONG>(scene,
				glm::vec3(10.0, 10.0, 10.0), glm::ivec2(0, 0),
				{{texIds[0], texIds[1], texIds[2],
				  texIds[3], texIds[4], texIds[5]}});

//		auto scene = Kaleidescope::buildKaleidescope<BLINN_PHONG>(std::array<unsigned, 3>{{4, 4, 2}}, 1.0f, 0.0f);

		rndr = new Renderer<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(
				scene, width, height,
				numReflectivePasses, maxViewDistance);

		arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);

		initGL();
	}

	void render(int delta) {
		rndr->renderToTexture(renderTex, mat4ToFloat16(camera.getViewMatrix()));

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
	}

	void update(int delta, SDL_Event& evt) {
		camera.update(evt);
		if(camera.isMoving()) {
			SDL_SetCursor(hand);
		} else {
			SDL_SetCursor(arrow);
		}
	}
};


int main(int argc, char* argv[]) {
	SDLApp<App> app(App::width, App::height, "Ray Tracer Demo");
	app.renderInteractive();

	return 0;
}
