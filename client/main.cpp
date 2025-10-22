#include "../common/constants.h"

#include <iostream>
#include <exception>
#include <SDL2/SDL.h>

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL_ttf.h>

#include "client_protocol.h"

using namespace SDL2pp;

int main() try {
	// Initialize SDL library
	SDL sdl(SDL_INIT_VIDEO);

	// Create main window: 640x480 dimensions, resizable, "SDL2pp demo" title
	Window window("SDL2pp demo",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_RESIZABLE);

	// Create accelerated video renderer with default driver
	Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

	Surface sprite_cars(DATA_PATH "/Mobile - Grand Theft Auto 4 - Miscellaneous - Cars.png");
	const Uint32 color_key = SDL_MapRGB(sprite_cars.Get()->format, 163, 163, 13);
	sprite_cars.SetColorKey(true, color_key);
	Texture sprites(renderer, sprite_cars);

	int pos_x = 300;
	int pos_y = 200;

	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return 0;
			} else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: case SDLK_q:
						return 0;
					case SDLK_w: pos_y -= 1; break;
					case SDLK_s: pos_y += 1; break;
					case SDLK_a: pos_x -= 1; break;
					case SDLK_d: pos_x += 1; break;
				}
			}
		}

		renderer.Clear();

		renderer.Copy(sprites,
			Rect(128, 32, 32, 32),
			Rect(pos_x, pos_y, 50, 50)
			);

		renderer.Present();

		SDL_Delay(100);
	}
} catch (std::exception& e) {
	// If case of error, print it and exit with error
	std::cerr << e.what() << std::endl;
	return 1;
}
