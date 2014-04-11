/*
 * sdl_app.h
 *
 *  Created on: Apr 10, 2014
 *      Author: francis
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <sstream>
#include <unistd.h>
#ifndef SDL_APP_H_
#define SDL_APP_H_

template <typename APP>
class SDLApp : public APP {
public:
	SDLApp(unsigned vpWidth, unsigned vpHeight,
			std::string title = "", bool showFPS = true) :
				title(title), showFps(showFPS){
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_CreateWindowAndRenderer(
				vpWidth, vpHeight, SDL_WINDOW_OPENGL,
				&window, &renderer);
		app.init();
	}

	void renderInteractive() {
		mainLoop(true);
	}

	void renderSingleFrame() {
		mainLoop(false);
	}

private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	std::string title;
	bool showFps;

	APP& app = *this;

	void mainLoop(bool interactive) {
		bool loop = true;
		int lastTicks = SDL_GetTicks();\
		bool first = true;

		while(loop) {
			int delta = SDL_GetTicks() - lastTicks;
			lastTicks = SDL_GetTicks();
			SDL_Event e;
			while(SDL_PollEvent(&e)) {
				if(e.type == SDL_QUIT) {
					loop = false;
				}

				app.update(delta, e);

				usleep(1000);
			}

			if(first) {
				app.render(delta);
				SDL_GL_SwapWindow(window);
				first = interactive;
			}

			std::stringstream ss;
			ss << title;
			if(!title.empty() && showFps) {
				ss << " (" << 1000.0f / delta << " FPS)";
			} else if(showFps) {
				ss << 1000.0f / delta;
			}

			SDL_SetWindowTitle(window, ss.str().c_str());
		}

		SDL_Quit();
	}
};



#endif /* SDL_APP_H_ */
