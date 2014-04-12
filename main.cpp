#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

#include "renderer/renderer.h"
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

	template <typename RandomAccessIterator>
	RandomAccessIterator cyclic_next(RandomAccessIterator it, RandomAccessIterator cycleBegin, RandomAccessIterator cycleEnd){
		if(it == cycleEnd - 1) {
			return cycleBegin;
		} else {
			return it + 1;
		}
	}

	std::vector<Triangle> triangleFromAngles(const std::array<unsigned, 3>& kpoints, float scale, float depth) {
		auto minAngleIter = std::max_element(kpoints.begin(), kpoints.end());
		auto nextAngleIter = minAngleIter;

		float minAngle = glm::pi<float>() / static_cast<float>(*minAngleIter);
		std::vector<Triangle> tris;
		Triangle t;

		glm::mat4 rotate = glm::rotate(glm::mat4(1.0), minAngle, glm::vec3(0.0, 0.0, 1.0));
		glm::mat4 rotate2 = glm::rotate(glm::mat4(1.0), 2.0f * minAngle, glm::vec3(0.0, 0.0, 1.0));
		glm::mat4 invRotate = glm::rotate(glm::mat4(1.0), -minAngle, glm::vec3(0.0, 0.0, 1.0));

		float sinA = glm::sin(minAngle);

		t.v1 = cl_float3{{0.0f, 0.0f, depth}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		std::cout << *nextAngleIter << std::endl;
		glm::vec2 v2 = glm::vec2(scale*glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA, 0.0);
		t.v2 = cl_float3{{v2.x, v2.y, depth}};

		nextAngleIter = cyclic_next(nextAngleIter, kpoints.begin(), kpoints.end());
		std::cout << *nextAngleIter << std::endl;
		glm::vec4 direction = rotate * glm::vec4(1.0, 0.0, 0.0, 0.0);
		std::cout << glm::to_string(direction) << std::endl;
		glm::vec2 v3 = glm::vec2(direction) * scale * (glm::sin(glm::pi<float>() / static_cast<float>(*nextAngleIter)) / sinA);
		t.v3 = cl_float3{{v3.x, v3.y, depth}};

		glm::vec3 n(0.0, 0.0, 1.0);
		t.normal = cl_float3{{n.x, n.y, n.z}};
		t.materialId = 0;

		tris.push_back(t);


		glm::mat4 tx = rotate * glm::scale(glm::mat4(1.0), glm::vec3(1.0, -1.0, 1.0)) * invRotate;

		Triangle t2 = t;
		t2.materialId = 1;
		transform<Triangle>(t2, tx);
		tris.push_back(t2);

		for(unsigned i = 0; i < *minAngleIter; i++) {
			Triangle t3= tris[tris.size()-2];
			Triangle t4 = tris[tris.size()-1];
			t3.materialId = 0;
			t4.materialId = 1;

			transform<Triangle>(t3, rotate2);
			transform<Triangle>(t4, rotate2);

			tris.push_back(t3);
			tris.push_back(t4);
		}


		return tris;
	}


	void init() {
		TextureArray textures;

		auto texIds = textures.createTextures<6>(
				{gli::load_dds("textures/tex1.dds"),
				 gli::load_dds("textures/tex2.dds"),
				 gli::load_dds("textures/tex3.dds"),
				 gli::load_dds("textures/bricks.dds"),
				 gli::load_dds("textures/bricks_normals.dds"),
				 gli::load_dds("textures/tex4.dds")});


		std::vector<Triangle> tris = (triangleFromAngles(std::array<unsigned, 3>{{2, 2, 8}}, 1.0, 0.0));

		std::vector<PointLight> lights;
		lights.push_back(PointLight{ cl_float3{{0.0f, 0.0f, 5.0f}}, cl_float3{{0.7f, 0.7f, 0.7f}}, 0.01f });

		std::vector<Material<BLINN_PHONG>> mats;
		mats.push_back(
				Material<BLINN_PHONG>{
					cl_float3{{0.0f, 0.0f, 0.0f}},
					cl_float3{{0.0f, 0.0f, 0.0f}},
					cl_float3{{0.5f, 0.5f, 0.5f}},
					cl_float3{{0.0f, 0.0f, 0.0f}},
					1.0, no_texture
				});
		mats.push_back(
				Material<BLINN_PHONG>{
					cl_float3{{0.0f, 0.0f, 0.0f}},
					cl_float3{{0.0f, 0.0f, 0.0f}},
					cl_float3{{0.0f, 0.5f, 0.5f}},
					cl_float3{{0.0f, 0.0f, 0.0f}},
					1.0, no_texture
				});

		std::vector<Sphere> spheres;

		Scene<BLINN_PHONG> s {tris, spheres, lights, mats};

		auto scene = TiledMirrorBox::buildTiledMirrorBox<BLINN_PHONG>(
				glm::vec3(10.0, 10.0, 10.0), glm::ivec2(0, 0),
				{{texIds[0], texIds[1], texIds[2],
				  texIds[3], texIds[4], texIds[5]}});

		rndr = new Renderer<CL_DEVICE_TYPE_GPU, BLINN_PHONG>(
				s, textures, width, height,
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
