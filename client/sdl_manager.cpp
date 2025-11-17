#include "sdl_manager.h"

#include <fstream>
#include <map>

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

SDLManager::SDLManager():
        sdl(SDL_INIT_VIDEO),
        mix(MIX_INIT_OGG),
        mixer(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024),
        window("NFS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080,
               SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        camera(512, 288, 4640, 4672),
        sounds(mixer) {
    renderer.SetLogicalSize(1920, 1080);
    set_map();
    set_sprite();
    set_minimap();
    load_bridges_rects(load_json_file("server/resources/world/bridges_liberty_city.json"));
    load_decorations_rects(load_json_file("server/resources/world/decorations_liberty_city.json"));
}

void SDLManager::set_map() {
    // Cargar imagen de fondo (circuito)
    const SDL2pp::Surface background_surface("client/assets/sprites/Game Boy _ GBC - Grand Theft "
                                             "Auto - Backgrounds - Liberty City.png");
    map = SDL2pp::Texture(renderer, background_surface);
}

void SDLManager::set_sprite() {
    SDL2pp::Surface sprite_cars("client/assets/sprites/cars.png");
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

std::vector<SDL2pp::Rect> SDLManager::loadRects(const nlohmann::json& data) {
    std::vector<SDL2pp::Rect> rects;
    for (const auto& body: data) {
        if (body.size() == 4) {
            int x = body[0][0];
            int y = body[0][1];
            int w = body[1][0].get<int>() - x;
            int h = body[2][1].get<int>() - y;

            rects.emplace_back(x, y, w, h);
        }
    }
    return rects;
}

void SDLManager::update_camera(const int player_x, const int player_y) {
    camera.update(player_x, player_y);
}

void SDLManager::render_player(const CarPositionDTO& player, const std::string& model,
                               bool is_moving) {
    SDL2pp::Rect view_rect = camera.get_view_rect();
    const int car_screen_x = (player.x_position - view_rect.x) * 1920 / view_rect.w;
    const int car_screen_y = (player.y_position - view_rect.y) * 1080 / view_rect.h;
    SDL2pp::Rect sprite_rect = SDL2pp::Rect(128, 32, 32, 32);  // default

    auto it = CAR_SPRITES.find(model);
    if (it != CAR_SPRITES.end()) {
        sprite_rect = it->second;
    }

    // Usar solo sprite vertical y rotarlo con SDL
    renderer.Copy(sprite.value(), sprite_rect,
                  SDL2pp::Rect(car_screen_x - 32, car_screen_y - 32, 84, 84),
                  static_cast<double>(player.angle), SDL2pp::NullOpt, SDL_FLIP_NONE);

    sounds.play_engine_sound(is_moving);
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

void SDLManager::render_layer(const std::vector<SDL2pp::Rect>& rects) {
    const SDL2pp::Rect view = camera.get_view_rect();
    constexpr int screen_w = 1920;
    constexpr int screen_h = 1080;

    for (const auto& src_rect: rects) {
        if (!camera.on_screen(src_rect))
            continue;

        // Vista en pantalla
        const int dx = (src_rect.x - view.x) * screen_w / view.w;
        const int dy = (src_rect.y - view.y) * screen_h / view.h;
        const int dw = src_rect.w * screen_w / view.w;
        const int dh = src_rect.h * screen_h / view.h;
        const SDL2pp::Rect dst_rect(dx, dy, dw, dh);

        renderer.Copy(map.value(), src_rect, dst_rect);
    }
}

void SDLManager::render_bridges() { render_layer(bridges_rects); }

void SDLManager::render(const PlayerDTO& player, const int health,
                        const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& cars,
                        const bool is_moving) {
    CarPositionDTO my_car;
    bool found = false;

    for (const auto& [car_player, car_pos]: cars) {
        if (car_player.id == player.id) {
            my_car = car_pos;
            found = true;
            break;
        }
    }

    if (!found) {
        SDL_Delay(10);
        return;
    }

    const uint32_t pos_x = my_car.x_position;
    const uint32_t pos_y = my_car.y_position;

    update_camera(pos_x, pos_y);
    clear_screen();

    renderer.Copy(map.value(), camera.get_view_rect(), SDL2pp::Rect(0, 0, 1920, 1080));

    for (const auto& [car_player, car_pos]: cars) {
        render_player(car_pos, car_player.car.model, is_moving);
    }

    if (my_car.layer != CarLayer::ON_BRIDGE) {
        render_bridges();
    }

    render_decorations();

    render_hud_health(health);
    render_hud_minimap(pos_x, pos_y);
    render_stats();

    show_screen();

    SDL_Delay(10);
}

void SDLManager::draw_row(const std::string& sentence, int y, SDL2pp::Color color,
                          int size_container) {
    int font_size = size_container / 10;
    int text_x = size_container + 10;
    SDL2pp::Font font("client/assets/fonts/UrbanHaze.ttf", font_size);
    SDL2pp::Surface surface(font.RenderText_Blended(sentence, color));
    SDL2pp::Texture text_texture(renderer, surface);
    renderer.Copy(text_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(text_x, y, text_texture.GetWidth(), text_texture.GetHeight()));
}

void SDLManager::render_stats() {
    int box_w = 1920 * 4 / 5;
    int box_h = 1080 * 4 / 5;
    int box_x = (1920 - box_w) / 2;
    int box_y = (1080 - box_h) / 2;

    SDL2pp::Rect stats{box_x, box_y, box_w, box_h};

    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, 100);
    renderer.DrawRect(stats);

    // Hardcodeado (temporal)
    std::string client_id = "Client ID: 007";
    std::string final_time = "Tiempo final: 02:34.56";

    SDL2pp::Color text_color = SDL2pp::Color(255, 255, 255, 255);

    int text_start_y = box_y + 10;
    int line_spacing = (box_h / 10) + 10;
    draw_row(client_id, text_start_y, text_color, box_x + box_w / 2);
    draw_row(final_time, text_start_y + line_spacing, text_color, box_x + box_w / 2);
}

void SDLManager::render_decorations() { render_layer(decorations_rects); }

void SDLManager::load_bridges_rects(const nlohmann::json& bridges_rects_data) {
    bridges_rects = loadRects(bridges_rects_data);
}

void SDLManager::load_decorations_rects(const nlohmann::json& decorations_rects_data) {
    decorations_rects = loadRects(decorations_rects_data);
}

nlohmann::json SDLManager::load_json_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open JSON file: " + path);
    }

    nlohmann::json data;
    file >> data;
    return data;
}

