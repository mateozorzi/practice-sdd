#include "client.h"

#include <any>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/car_position_dto.h"
#include "../common/movement_dto.h"

#include "camera.h"
#include "menu_interface.h"

// NOLINTNEXTLINE(build/namespaces)
using namespace SDL2pp;

Client::Client(const std::string& hostname, const std::string& servname):
        protocol(hostname, servname),
        receiver(protocol),
        input_manager(protocol),
        menu(protocol),
        client_id(0) {}

void Client::run() {
    // lobby ui
    
    if (menu.lobby()) {
        exit(0);
    }
    

    protocol.send_username("a");

    // Initialize SDL library
    SDL sdl(SDL_INIT_VIDEO);

    // Ventana principal con resolución 1920 x 1080
    Window window("SDL2pp demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080,
                  SDL_WINDOW_RESIZABLE);

    // Create accelerated video renderer with default driver
    Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    renderer.SetLogicalSize(1920, 1080);

    // Cargar imagen de fondo (circuito)
    const Surface background_surface(
            DATA_PATH "/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Liberty City.png");
    Texture background(renderer, background_surface);

    // Obtener dimensiones del fondo
    const int bg_width = background_surface.GetWidth();
    const int bg_height = background_surface.GetHeight();

    // Cargar minimapa
    constexpr int minimap_width = 384;   // 1920 : 5
    constexpr int minimap_height = 216;  // 1080 : 5
    Texture minimap_static(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                           minimap_width, minimap_height);
    renderer.SetTarget(minimap_static);
    renderer.Copy(background, Rect(0, 0, bg_width, bg_height),
                  Rect(0, 0, minimap_width, minimap_height));
    renderer.SetTarget();

    Surface sprite_cars(DATA_PATH "/Mobile - Grand Theft Auto 4 - Miscellaneous - Cars.png");
    const Uint32 color_key = SDL_MapRGB(sprite_cars.Get()->format, 163, 163, 13);
    sprite_cars.SetColorKey(true, color_key);
    Texture sprites(renderer, sprite_cars);

    // Vida del jugador (0-100)
    int health = 50;  // Para probar como baja la barra de vida

    // Camara
    constexpr int view_width = 512;
    constexpr int view_height = 288;
    Camera camera(view_width, view_height, bg_width, bg_height);

    // Recibir client_id del servidor antes de iniciar receiver
    client_id = protocol.receive_id();
    receiver.start();

    // ID - Posicion
    std::map<uint32_t, CarPositionDTO> cars;

    while (true) {
        if (!input_manager.handle_movement()) {
            return;
        }

        // Obtener posiciones
        Queue<std::vector<CarPositionDTO>>& car_queue = receiver.get_car_positions_queue();
        std::vector<CarPositionDTO> car_positions;
        bool has_positions = false;
        while (car_queue.try_pop(car_positions)) {
            has_positions = true;
        }
        if (has_positions) {
            std::vector<uint32_t> received_ids;
            for (CarPositionDTO& car_position: car_positions) {
                received_ids.push_back(car_position.client_id);
                cars[car_position.client_id] = car_position;
            }
            for (auto it = cars.begin(); it != cars.end();) {
                const bool found = std::ranges::any_of(
                        received_ids, [&](uint32_t id) { return id == it->first; });
                if (!found) {
                    it = cars.erase(it);
                } else {
                    ++it;
                }
            }
        }
        // Hallar posicion del cliente actual
        if (cars.contains(client_id)) {
            const CarPositionDTO& my_car = cars[client_id];
            const uint32_t pos_x = my_car.x_position;
            const uint32_t pos_y = my_car.y_position;

            // Seguir a cliente actual
            camera.update(pos_x, pos_y);

            renderer.Clear();

            // Dibujar elementos del juego
            draw_background(renderer, background, camera.get_view_rect());

            // Dibujar todos los autos
            for (const auto& car_pair: cars) {
                const CarPositionDTO& car = car_pair.second;
                draw_car(renderer, sprites, car.x_position, car.y_position, camera.get_view_rect(),
                         car.angle);
            }

            draw_health_bar(renderer, health);
            draw_minimap(renderer, minimap_static, pos_x, pos_y, bg_width, bg_height, minimap_width,
                         minimap_height);

            renderer.Present();
        }

        SDL_Delay(16);
    }
}

void Client::draw_background(Renderer& renderer, Texture& background, const Rect& view_rect) {
    renderer.Copy(background, view_rect, Rect(0, 0, 1920, 1080));
}

void Client::draw_car(Renderer& renderer, Texture& sprites, const int pos_x, const int pos_y,
                      const Rect& view_rect, const int angle) {
    const int car_screen_x = (pos_x - view_rect.x) * 1920 / view_rect.w;
    const int car_screen_y = (pos_y - view_rect.y) * 1080 / view_rect.h;

    // Usar solo sprite vertical y rotarlo con SDL
    renderer.Copy(sprites, Rect(128, 32, 32, 32),
                  Rect(car_screen_x - 32, car_screen_y - 32, 64, 64), angle, NullOpt,
                  SDL_FLIP_NONE);
}

void Client::draw_health_bar(Renderer& renderer, int health) {
    constexpr int health_bar_width = 400;
    constexpr int health_bar_height = 20;
    constexpr int health_bar_x = 20;
    constexpr int health_bar_y = 1080 - health_bar_height - 20;

    // Fondo de la barra
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.FillRect(Rect(health_bar_x, health_bar_y, health_bar_width, health_bar_height));

    // Dibujar barra de vida
    const int current_health_width = (health * health_bar_width) / 100;
    renderer.SetDrawColor(255, 0, 0, 255);
    renderer.FillRect(Rect(health_bar_x, health_bar_y, current_health_width, health_bar_height));
}

void Client::draw_minimap(Renderer& renderer, Texture& minimap_static, const int pos_x,
                          const int pos_y, const int bg_width, const int bg_height,
                          int minimap_width, int minimap_height) {
    const int minimap_x = 1920 - minimap_width - 20;
    const int minimap_y = 1080 - minimap_height - 20;

    // Borde del minimapa
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.FillRect(Rect(minimap_x - 2, minimap_y - 2, minimap_width + 4, minimap_height + 4));

    // Dibujar minimapa
    renderer.Copy(minimap_static, NullOpt,
                  Rect(minimap_x, minimap_y, minimap_width, minimap_height));

    // Jugador en el minimapa
    const int player_minimap_x = minimap_x + (pos_x * minimap_width / bg_width);
    const int player_minimap_y = minimap_y + (pos_y * minimap_height / bg_height);

    // Dibujar jugador en minimapa
    renderer.SetDrawColor(255, 255, 0, 255);
    renderer.FillRect(Rect(player_minimap_x - 2, player_minimap_y - 2, 4, 4));
}

Client::~Client() {
    if (receiver.is_alive()) {
        receiver.stop();
        receiver.join();
    }
}
