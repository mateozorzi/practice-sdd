#include "client.h"
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

using namespace SDL2pp;

Client::Client(const std::string &hostname, const std::string &servname):
    protocol(hostname, servname) {}

void Client::run() {
	// Initialize SDL library
	SDL sdl(SDL_INIT_VIDEO);

	// Create main window: 640x480 dimensions, resizable, "SDL2pp demo" title
	Window window("SDL2pp demo",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_RESIZABLE);

	// Create accelerated video renderer with default driver
	Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

	// Cargar imagen de fondo (circuito)
	const Surface background_surface(DATA_PATH "/circuito_liberty_city.png");
	Texture background(renderer, background_surface);

	// Obtener dimensiones del fondo
	const int bg_width = background_surface.GetWidth();
	const int bg_height = background_surface.GetHeight();

	Surface sprite_cars(DATA_PATH "/Mobile - Grand Theft Auto 4 - Miscellaneous - Cars.png");
	const Uint32 color_key = SDL_MapRGB(sprite_cars.Get()->format, 163, 163, 13);
	sprite_cars.SetColorKey(true, color_key);
	Texture sprites(renderer, sprite_cars);

	// Posición del vehiculo en el mundo
	int pos_x = 300;
	int pos_y = 200;

	// Desplazamiento de la cámara
	int camera_x = 0;
	int camera_y = 0;

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
				return;
			}
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: case SDLK_q:
						return;
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

		if (key_w && pos_y > 0) {
			pos_y -= 2;
			car_y = 32; // sprite arriba
		}
		if (key_s && pos_y < bg_height + 160) {
			pos_y += 2;
			car_y = 0; // sprite abajo
		}
		if (key_a && pos_x > 0) {
			pos_x -= 2;
			car_x = 96;  // sprite izquierda
		}
		if (key_d && pos_x < bg_width + 240) {
			pos_x += 2;
			car_x = 160; // sprite derecha
		}

		if (!key_a && !key_d) {
			car_x = 128;
		}

		// Actualizar camara siguiendo al auto
		camera_x = pos_x - 160 + 25;
		camera_y = pos_y - 120 + 25;

		// Limite de camara
		if (camera_x < 0) camera_x = 0;
		if (camera_y < 0) camera_y = 0;
		if (camera_x > bg_width - 320) camera_x = bg_width - 320;
		if (camera_y > bg_height - 240) camera_y = bg_height - 240;

		renderer.Clear();

		// Dibujar mapa
		renderer.Copy(background,
			Rect(camera_x, camera_y, 320, 240),
			Rect(0, 0, 640, 480)
			);

		// Dibujar auto
		renderer.Copy(sprites,
			Rect(car_x, car_y, 32, 32),
			Rect(pos_x - camera_x, pos_y - camera_y, 50, 50)
			);

		renderer.Present();

		SDL_Delay(10);
	}
}
