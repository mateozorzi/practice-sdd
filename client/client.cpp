#include "client.h"
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

using namespace SDL2pp;

Client::Client(const std::string &hostname, const std::string &servname):
    protocol(hostname, servname), sender(protocol), receiver(protocol) {
    sender.start();
    receiver.start();
}

void Client::run() {
	// Initialize SDL library
	SDL sdl(SDL_INIT_VIDEO);

	// Ventana principal con resolución 1920 x 1080
	Window window("SDL2pp demo",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			1920, 1080,
			SDL_WINDOW_RESIZABLE);

	// Create accelerated video renderer with default driver
	Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
	renderer.SetLogicalSize(1920, 1080);

	// Cargar imagen de fondo (circuito)
	const Surface background_surface(DATA_PATH "/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Liberty City.png");
	Texture background(renderer, background_surface);

	// Obtener dimensiones del fondo
	const int bg_width = background_surface.GetWidth();
	const int bg_height = background_surface.GetHeight();

	// Cargar minimapa
	const int minimap_width = 384; // 1920 : 5
	const int minimap_height = 216; // 1080 : 5
	Texture minimap_static(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, minimap_width, minimap_height);
	renderer.SetTarget(minimap_static);
	renderer.Copy(background,Rect(0, 0, bg_width, bg_height),
		Rect(0, 0, minimap_width, minimap_height));
	renderer.SetTarget();

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

	// Vida del jugador (0-100)
	int health = 50; // Para probar como baja la barra de vida

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
					case SDLK_w: key_w = true; protocol.send_car_movement(CarMovement::ACCELERATE); break;
					case SDLK_s: key_s = true; protocol.send_car_movement(CarMovement::BRAKE); break;
					case SDLK_a: key_a = true; protocol.send_car_movement(CarMovement::TURN_LEFT); break;
					case SDLK_d: key_d = true; protocol.send_car_movement(CarMovement::TURN_RIGHT); break;
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
	const int view_width = 512;
	const int view_height = 288;
	camera_x = pos_x - (view_width / 2) + 25;
	camera_y = pos_y - (view_height / 2) + 25;

	// Limite de camara
	if (camera_x < 0) camera_x = 0;
	if (camera_y < 0) camera_y = 0;
	if (camera_x > bg_width - view_width) camera_x = bg_width - view_width;
	if (camera_y > bg_height - view_height) camera_y = bg_height - view_height;

		renderer.Clear();

		// Dibujar elementos del juego
		draw_background(renderer, background, camera_x, camera_y, view_width, view_height);
		draw_car(renderer, sprites, pos_x, pos_y, camera_x, camera_y, car_x, car_y, view_width, view_height);
		draw_health_bar(renderer, health);
		draw_minimap(renderer, minimap_static, pos_x, pos_y, bg_width, bg_height, minimap_width, minimap_height);

		renderer.Present();

		SDL_Delay(10);
	}
}

void Client::draw_background(Renderer& renderer, Texture& background, int camera_x, int camera_y, int view_width, int view_height) {
	renderer.Copy(background,
		Rect(camera_x, camera_y, view_width, view_height),
		Rect(0, 0, 1920, 1080));
}

void Client::draw_car(Renderer& renderer, Texture& sprites, int pos_x, int pos_y, int camera_x, int camera_y, int car_x,
	int car_y, int view_width, int view_height) {
	int car_screen_x = (pos_x - camera_x) * 1920 / view_width;
	int car_screen_y = (pos_y - camera_y) * 1080 / view_height;

	renderer.Copy(sprites,Rect(car_x, car_y, 32, 32),
		Rect(car_screen_x, car_screen_y, 60, 60));
}

void Client::draw_health_bar(Renderer& renderer, int health) {
	const int health_bar_width = 400;
	const int health_bar_height = 20;
	const int health_bar_x = 20;
	const int health_bar_y = 1080 - health_bar_height - 20;

	// Fondo de la barra
	renderer.SetDrawColor(0, 0, 0, 255);
	renderer.FillRect(Rect(health_bar_x, health_bar_y, health_bar_width, health_bar_height));

	// Dibujar barra de vida
	int current_health_width = (health * health_bar_width) / 100;
	renderer.SetDrawColor(255, 0, 0, 255);
	renderer.FillRect(Rect(health_bar_x, health_bar_y, current_health_width, health_bar_height));
}

void Client::draw_minimap(Renderer& renderer, Texture& minimap_static, int pos_x, int pos_y, int bg_width, int bg_height,
	int minimap_width, int minimap_height) {
	const int minimap_x = 1920 - minimap_width - 20;
	const int minimap_y = 1080 - minimap_height - 20;

	// Borde del minimapa
	renderer.SetDrawColor(0, 0, 0, 255);
	renderer.FillRect(Rect(minimap_x - 2, minimap_y - 2, minimap_width + 4, minimap_height + 4));

	// Dibujar minimapa
	renderer.Copy(minimap_static,NullOpt, Rect(minimap_x, minimap_y, minimap_width, minimap_height));

	// Jugador en el minimapa
	int player_minimap_x = minimap_x + (pos_x * minimap_width / bg_width);
	int player_minimap_y = minimap_y + (pos_y * minimap_height / bg_height);

	// Dibujar jugador en minimapa
	renderer.SetDrawColor(255, 255, 0, 255);
	renderer.FillRect(Rect(player_minimap_x - 2, player_minimap_y - 2, 4, 4));
}

Client::~Client() {
	if (sender.is_alive()) {
		sender.stop();
		sender.join();
	}
	if (receiver.is_alive()) {
		receiver.stop();
		receiver.join();
	}
}

