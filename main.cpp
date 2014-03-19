#define BLINN_PHONG

#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "multi_pass_renderer.h"
#include "single_pass_renderer.h"
#include "scene.h"

//#define RENDER_LIGHTS

using namespace std;

const int kWidth = 640;
const int kHeight = 480;
const int numReflectivePasses = 3;
const double maxViewDistance = 15.0;
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glFinish();	// Texture needs to exist for openCL
}

std::shared_ptr<Scene<CL_DEVICE_TYPE_GPU>> initScene() {
	float planeWidth = 100.0;
	float planeHeight = 100.0;
	float planeSeparation = 5.0;
	unsigned trisX = 1;
	unsigned trisY = 1;

	float halfSeperation = planeSeparation/2.0;
	float halfWidth = planeWidth/2.0;
	float halfHeight = planeHeight/2.0;
	float dX = planeWidth/trisX;
	float dY = planeWidth/trisY;

	for(unsigned i = 0; i < trisX; i++) {
		for(unsigned j = 0; j < trisY; j++) {
			// Top plane
			tris.push_back(
				Triangle{
					cl_float3{{-halfWidth+i*dX,     halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+i*dX,     halfSeperation, -halfHeight+(j+1)*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, halfSeperation, -halfHeight+(j+1)*dY}},
					2});
			tris.push_back(
				Triangle{
					cl_float3{{-halfWidth+i*dX,     halfSeperation, -halfHeight+j*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, halfSeperation, -halfHeight+(j+1)*dY}},
					cl_float3{{-halfWidth+(i+1)*dX, halfSeperation, -halfHeight+j*dY}},
					2});

			// Bottom plane
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

	// Spheres
	spheres.push_back(Sphere{1.0, cl_float3{{-2.5, -0.0, -3.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{ 2.5, -0.0, -3.0}}, 1});

	spheres.push_back(Sphere{1.0, cl_float3{{-2.4, -0.0, -5.0}}, 1});
	spheres.push_back(Sphere{1.0, cl_float3{{ 2.4, -0.0, -5.0}}, 0});

	spheres.push_back(Sphere{1.0, cl_float3{{-2.3, -0.0, -7.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{ 2.3, -0.0, -7.0}}, 1});

	spheres.push_back(Sphere{1.0, cl_float3{{-2.2, -0.0, -9.0}}, 1});
	spheres.push_back(Sphere{1.0, cl_float3{{ 2.2, -0.0, -9.0}}, 0});

	spheres.push_back(Sphere{1.0, cl_float3{{-2.1, -0.0, -11.0}}, 0});
	spheres.push_back(Sphere{1.0, cl_float3{{ 2.1, -0.0, -11.0}}, 1});

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

#ifdef BLINN_PHONG
	// Materials
	mats.push_back(
			Material{
				cl_float3{{0.005, 0.005, 0.005}},
				cl_float3{{0.6, 0.6, 0.6}},
				cl_float3{{0.5, 0.5, 0.5}},
				10000.0
	});

	mats.push_back(
			Material{
				cl_float3{{0.05, 0.05, 0.05}},
				cl_float3{{0.6, 0.4, 0.4}},
				cl_float3{{0.5, 0.5, 0.5}},
				100.0});

	mats.push_back(
			Material{
				cl_float3{{0.99, 0.99, 0.99}},
				cl_float3{{0.3, 0.3, 0.31}},
				cl_float3{{0.3, 0.3, 0.3}},
				1000.0});

	mats.push_back(
			Material{
				cl_float3{{0.99, 0.99, 0.99}},
				cl_float3{{0.3, 0.31, 0.3}},
				cl_float3{{0.5, 0.5, 0.5}},
				1000.0});
#else
	// Materials
	mats.push_back(
			Material{
				cl_float3{{0.2, 0.2, 0.2}},
				cl_float3{{0.1, 0.1, 0.6}}});

	mats.push_back(
			Material{
				cl_float3{{0.2, 0.2, 0.2}},
				cl_float3{{0.6, 0.1, 0.1}}});

	mats.push_back(
			Material{
				cl_float3{{0.99, 0.99, 0.99}},
				cl_float3{{0.3, 0.3, 0.3}}});

	mats.push_back(
			Material{
				cl_float3{{0.99, 0.99, 0.99}},
				cl_float3{{0.3, 0.3, 0.3}}});
#endif

	auto devCtx = std::make_shared<ClDeviceContext<CL_DEVICE_TYPE_GPU>>();
	return std::make_shared<Scene<CL_DEVICE_TYPE_GPU>>(
					devCtx, spheres.begin(), spheres.end(),
					tris.begin(), tris.end(),
					lights.begin(), lights.end(),
					mats.begin(), mats.end());
}

template <typename Renderer>
void render(int delta, Renderer& rndr) {
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
	glLoadIdentity();
	glFrustum(-0.5, 0.5, -0.5, 0.5, 0.5, 100.0);

	glPushAttrib(GL_CURRENT_BIT);

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(auto i = lights.begin(); i != lights.end(); i++) {
		glColor3fv((GLfloat*)&i->power);
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
	auto rndr = SinglePassRenderer<CL_DEVICE_TYPE_GPU>(initScene(), kWidth, kHeight, numReflectivePasses, maxViewDistance);

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
		}

		update(delta);
		render(delta, rndr);

		std::stringstream ss;
		ss << 1000.0f / delta;
		SDL_WM_SetCaption(ss.str().c_str(), 0);
	}

	return 0;
}
