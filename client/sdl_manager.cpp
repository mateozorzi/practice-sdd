#include "sdl_manager.h"

#include <map>

#include <SDL2/SDL.h>

SDLManager::SDLManager():
        sdl(SDL_INIT_VIDEO),
        window("NFS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080,
               SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        camera(512, 288, 4640, 4672) {
    renderer.SetLogicalSize(1920, 1080);
    set_map();
    set_sprite();
    set_minimap();
}

void SDLManager::set_map() {
    // Cargar imagen de fondo (circuito)
    const SDL2pp::Surface background_surface(
            DATA_PATH "/Game Boy _ GBC - Grand Theft Auto - Backgrounds - Liberty City.png");
    map = SDL2pp::Texture(renderer, background_surface);
}

void SDLManager::set_sprite() {
    SDL2pp::Surface sprite_cars(DATA_PATH
                                "/Mobile - Grand Theft Auto 4 - Miscellaneous - Cars.png");
    const Uint32 color_key = SDL_MapRGB(sprite_cars.Get()->format, 163, 163, 13);
    sprite_cars.SetColorKey(true, color_key);
    sprite = SDL2pp::Texture(renderer, sprite_cars);
}

void SDLManager::set_minimap() {
    constexpr int minimap_width = 384;   // 1920 : 5
    constexpr int minimap_height = 216;  // 1080 : 5
    minimap = SDL2pp::Texture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                              minimap_width, minimap_height);
    renderer.SetTarget(minimap.value());
    renderer.Copy(map.value(), SDL2pp::Rect(0, 0, 4640, 4672),
                  SDL2pp::Rect(0, 0, minimap_width, minimap_height));
    renderer.SetTarget();
}


void SDLManager::clear_screen() { renderer.Clear(); }

void SDLManager::show_screen() { renderer.Present(); }

void SDLManager::update_camera(const int player_x, const int player_y) {
    camera.update(player_x, player_y);
}

void SDLManager::render_player(CarPositionDTO& player) {
    SDL2pp::Rect view_rect = camera.get_view_rect();
    const int car_screen_x = (player.x_position - view_rect.x) * 1920 / view_rect.w;
    const int car_screen_y = (player.y_position - view_rect.y) * 1080 / view_rect.h;

    // Usar solo sprite vertical y rotarlo con SDL
    renderer.Copy(sprite.value(), SDL2pp::Rect(128, 32, 32, 32),
                  SDL2pp::Rect(car_screen_x - 32, car_screen_y - 32, 64, 64),
                  static_cast<double>(player.angle), SDL2pp::NullOpt, SDL_FLIP_NONE);
}

void SDLManager::render_hud_health(const int health) {
    constexpr int health_bar_width = 400;
    constexpr int health_bar_height = 20;
    constexpr int health_bar_x = 20;
    constexpr int health_bar_y = 1080 - health_bar_height - 20;

    // Fondo de la barra
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.FillRect(
            SDL2pp::Rect(health_bar_x, health_bar_y, health_bar_width, health_bar_height));

    // Dibujar barra de vida
    const int current_health_width = (health * health_bar_width) / 100;
    renderer.SetDrawColor(255, 0, 0, 255);
    renderer.FillRect(
            SDL2pp::Rect(health_bar_x, health_bar_y, current_health_width, health_bar_height));
}

void SDLManager::render_hud_minimap(int player_x, int player_y) {
    constexpr int minimap_width = 384;   // 1920 : 5
    constexpr int minimap_height = 216;  // 1080 : 5

    const int minimap_x = 1920 - minimap_width - 20;
    const int minimap_y = 1080 - minimap_height - 20;

    // Borde del minimapa
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.FillRect(
            SDL2pp::Rect(minimap_x - 2, minimap_y - 2, minimap_width + 4, minimap_height + 4));

    // Dibujar minimapa
    renderer.Copy(minimap.value(), SDL2pp::NullOpt,
                  SDL2pp::Rect(minimap_x, minimap_y, minimap_width, minimap_height));

    // Jugador en el minimapa
    const int player_minimap_x = minimap_x + (player_x * minimap_width / 4640);
    const int player_minimap_y = minimap_y + (player_y * minimap_height / 4672);

    // Dibujar jugador en minimapa
    renderer.SetDrawColor(255, 255, 0, 255);
    renderer.FillRect(SDL2pp::Rect(player_minimap_x - 2, player_minimap_y - 2, 10, 10));
}

void SDLManager::render(const uint32_t client_id, const int health,
                        std::map<uint32_t, CarPositionDTO> cars) {
    if (cars.contains(client_id)) {
        const CarPositionDTO& my_car = cars[client_id];
        const uint32_t pos_x = my_car.x_position;
        const uint32_t pos_y = my_car.y_position;

        // Seguir a cliente actual
        update_camera(pos_x, pos_y);
        clear_screen();

        renderer.Copy(map.value(), camera.get_view_rect(), SDL2pp::Rect(0, 0, 1920, 1080));

        // Dibujar todos los autos
        for (const auto& car_pair: cars) {
            CarPositionDTO car = car_pair.second;
            render_player(car);
        }

        render_hud_health(health);
        render_hud_minimap(pos_x, pos_y);
        show_screen();
    }
    SDL_Delay(16);
}
