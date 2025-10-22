#include "../common/constants.h"

#include <iostream>
#include <exception>

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL_ttf.h>

#include "client_protocol.h"

using namespace SDL2pp;

int main() try {
	// Initialize SDL library
	SDL sdl(SDL_INIT_VIDEO);
	TTF_Init();

	// Create main window: 640x480 dimensions, resizable, "SDL2pp demo" title
	Window window("SDL2pp demo",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_RESIZABLE);

	// Create accelerated video renderer with default driver
	Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 24);

	bool running = true;
	SDL_Event event;
	std::string current_event = "Esperando evento...";

	// Creo conexion a servidor
	ClientProtocol client("192.0.0.1", "8080");

	// Loop para leer event keys del teclado
	while (running) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
				case SDLK_w:
						current_event = "Avanzar";
						client.send_car_movement(CarMovement::ACCELERATE);
						break;
					case SDLK_s:
						current_event = "Frenar";
						client.send_car_movement(CarMovement::BRAKE);
						break;
					case SDLK_a:
						current_event = "Girando a la izquierda";
						client.send_car_movement(CarMovement::TURN_LEFT);
						break;
					case SDLK_d:
						current_event = "Girando a la derecha";
						client.send_car_movement(CarMovement::TURN_RIGHT);
						break;
						default:
							current_event = "Tecla no manejada";
							break;
					}
					break;
				default:
					break;
			}
		}

		renderer.SetDrawColor(0, 0, 0, 255);
		renderer.Clear();

		// Renderizar el texto
		SDL_Color textColor = {255, 255, 255, 255};
		SDL_Surface* textSurfacePtr = TTF_RenderText_Solid(font, current_event.c_str(), textColor);
		if (textSurfacePtr) {
			Surface textSurface(textSurfacePtr);
			Texture text(renderer, textSurface);
			renderer.Copy(text, NullOpt, Rect(Point(0, 0), text.GetSize()));
		}
		renderer.Present();
		SDL_Delay(100);
	}
	return 0;
} catch (std::exception& e) {
	// If case of error, print it and exit with error
	std::cerr << e.what() << std::endl;
	return 1;
}
