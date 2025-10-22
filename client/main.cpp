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

	// Posición del vehiculo
	int pos_x = 300;
	int pos_y = 200;

	// Sprite del vehiculo
	int car_x = 128;
	int car_y = 32;

	// Estado de WASD
	bool key_w = false;
	bool key_a = false;
	bool key_s = false;
	bool key_d = false;

	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return 0;
			}
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: case SDLK_q:
						return 0;
					case SDLK_w: key_w = true; break;
					case SDLK_s: key_s = true; break;
					case SDLK_a: key_a = true; break;
					case SDLK_d: key_d = true; break;
				}
			} else if (event.type == SDL_KEYUP) {
				switch (event.key.keysym.sym) {
					case SDLK_w: key_w = false; break;
					case SDLK_s: key_s = false; break;
					case SDLK_a: key_a = false; break;
					case SDLK_d: key_d = false; break;
				}
			}
		}

		if (key_w) {
			pos_y -= 2;
			car_y = 32; // sprite arriba
		}
		if (key_s) {
			pos_y += 2;
			car_y = 0; // sprite abajo
		}
		if (key_a) {
			pos_x -= 2;
			car_x = 96;  // sprite izquierda
		}
		if (key_d) {
			pos_x += 2;
			car_x = 160; // sprite derecha
		}

		if (!key_a && !key_d) {
			car_x = 128;
		}

		renderer.Clear();

		renderer.Copy(sprites,
			Rect(car_x, car_y, 32, 32),
			Rect(pos_x, pos_y, 50, 50)
			);

		renderer.Present();

		SDL_Delay(10);
	}
} catch (std::exception& e) {
	// If case of error, print it and exit with error
	std::cerr << e.what() << std::endl;
	return 1;
}
