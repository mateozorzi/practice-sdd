#include "sdl_manager.h"

#include <algorithm>
#include <fstream>
#include <map>

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <qforeach.h>

#include "game_state.h"

SDLManager::SDLManager():
        sdl(SDL_INIT_VIDEO),
        mix(MIX_INIT_OGG),
        mixer(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024),
        window("NFS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080,
               SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        resources(renderer),
        camera(512, 288, 4640, 4672),
        sounds(mixer, resources),
        animations(renderer, camera, resources),
        shop(renderer, mixer, resources, sounds, 1920, 1080, "client/assets/fonts/UrbanHaze.ttf") {
    renderer.SetLogicalSize(1920, 1080);
    load_city("Liberty City");
}

void SDLManager::set_minimap() {
    constexpr int minimap_width = 384;   // 1920 : 5
    constexpr int minimap_height = 216;  // 1080 : 5
    minimap = SDL2pp::Texture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                              minimap_width, minimap_height);
    renderer.SetTarget(minimap.value());
    std::cout << "Cargando minimap de " << map_name << std::endl;
    SDL2pp::Texture& map = resources.get_texture(map_name);
    renderer.Copy(map, SDL2pp::Rect(0, 0, 4640, 4672),
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
    SDL2pp::Texture& cars = resources.get_texture("client/assets/sprites/cars-no-background.png");
    renderer.Copy(cars, sprite_rect, SDL2pp::Rect(car_screen_x - 50, car_screen_y - 50, 100, 100),
                  static_cast<double>(player.angle), SDL2pp::NullOpt, SDL_FLIP_NONE);

    sounds.play_engine_sound(is_moving);
}

void SDLManager::render_hud_health(const int health, const int max_health) {
    constexpr int health_bar_width = 400;
    constexpr int health_bar_height = 20;
    constexpr int health_bar_x = 20;
    constexpr int health_bar_y = 1080 - health_bar_height - 20;

    // Fondo de la barra
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.FillRect(
            SDL2pp::Rect(health_bar_x, health_bar_y, health_bar_width, health_bar_height));

    // Dibujar barra de vida segun vida maxima del auto
    int current_health_width = 0;
    if (max_health > 0) {
        current_health_width = (health * health_bar_width) / max_health;
        if (current_health_width > health_bar_width) {
            current_health_width = health_bar_width;
        }
    }
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

        SDL2pp::Texture& map = resources.get_texture(map_name);
        renderer.Copy(map, src_rect, dst_rect);
    }
}

void SDLManager::render_bridges() { render_layer(bridges_rects); }

void SDLManager::render(LocalInfo& game_state) {
    iteration_counter++;

    std::vector<std::pair<PlayerDTO, CarPositionDTO>> cars;

    for (const auto& [pid, pinfo]: game_state.players) {
        PlayerDTO p;
        p.id = pid;
        p.car.model = pinfo.model;

        CarPositionDTO pos;
        pos.x_position = pinfo.x;
        pos.y_position = pinfo.y;
        pos.angle = pinfo.angle;
        pos.layer = pinfo.layer;
        pos.next = pinfo.checkpoint;

        cars.emplace_back(p, pos);
    }

    CarPositionDTO my_car;
    bool found = false;
    uint32_t my_id = game_state.id;

    for (const auto& [car_player, car_pos]: cars) {
        if (car_player.id == my_id) {
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

    /* if (!animations.is_explosion_active(iteration_counter)) {
        animations.start_explosion(iteration_counter,
                                   SDL2pp::Point(pos_x, pos_y));
    } */

    update_camera(pos_x, pos_y);
    clear_screen();

    SDL2pp::Texture& map = resources.get_texture(map_name);
    renderer.Copy(map, camera.get_view_rect(), SDL2pp::Rect(0, 0, 1920, 1080));

    for (const auto& [car_player, car_pos]: cars) {
        if (car_pos.layer != CarLayer::ON_BRIDGE) {
            render_player(car_pos, car_player.car.model, true);
        }
    }

    render_bridges();

    for (const auto& [car_player, car_pos]: cars) {
        if (car_pos.layer == CarLayer::ON_BRIDGE) {
            render_player(car_pos, car_player.car.model, true);
        }
    }

    render_decorations();

    render_checkpoints(my_car.next);

    render_hud_health(game_state.player.life, game_state.player.max_life);
    render_hud_minimap(pos_x, pos_y);
    render_stats(cars);
    render_hint(my_car);

    // animations.render_explosion(iteration_counter);

    show_screen();
    SDL_Delay(10);
}

void SDLManager::draw_row(const std::string& sentence, int x, int y, SDL2pp::Color color,
                          int font_size) {

    SDL2pp::Texture& text_texture =
            resources.get_text(sentence, "client/assets/fonts/UrbanHaze.ttf", font_size, color);

    renderer.Copy(text_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(x, y, text_texture.GetWidth(), text_texture.GetHeight()));
}

void SDLManager::render_stats(const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& cars) {
    int box_w = 1920 * 4 / 5;
    int box_h = 1080 * 4 / 5;
    int box_x = (1920 - box_w) / 2;
    int box_y = (1080 - box_h) / 2;

    SDL2pp::Rect stats{box_x, box_y, box_w, box_h};

    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, 100);
    renderer.DrawRect(stats);

    SDL2pp::Color text_color = SDL2pp::Color(255, 255, 255, 255);

    int margin = 20;
    int text_x = box_x + margin;
    int text_y = box_y + margin;
    int font_size = box_h / 20;
    int spacing = font_size + 10;

    std::string separation = "     ";
    std::string header = "Jugador" + separation + "Tiempo";

    draw_row(header, text_x, text_y, text_color, font_size);

    for (const auto& [player, car_pos]: cars) {
        std::string name = player.username;
        std::string time = "N/A";
        std::string player_result = name + separation + time;
        draw_row(player_result, text_x, text_y + spacing, text_color, font_size);
    }
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

std::optional<ButtonType> SDLManager::interact_button(int x, int y, int time) {
    return shop.interact_button(x, y, time);
}

void SDLManager::render_shop(int time) { shop.render(time, Car::DEFAULT); }

void SDLManager::close_shop() { shop.close_shop(); }

void SDLManager::render_checkpoints(const CheckpointDTO& checkpoint) {
    SDL2pp::Rect view = camera.get_view_rect();

    SDL2pp::Rect checkpoint_rect(checkpoint.x, checkpoint.y, checkpoint.width, checkpoint.height);
    if (!camera.on_screen(checkpoint_rect)) {
        return;
    }

    int screen_x = ((checkpoint.x - checkpoint.width / 2) - view.x) * 1920 / view.w;
    int screen_y = ((checkpoint.y - checkpoint.height / 2) - view.y) * 1080 / view.h;
    int screen_w = checkpoint.width * 1920 / view.w;
    int screen_h = checkpoint.height * 1080 / view.h;

    SDL2pp::Rect dst(screen_x, screen_y, screen_w, screen_h);

    if (checkpoint.is_finish_line) {
        renderer.SetDrawColor(255, 255, 0, 200);
    } else {
        renderer.SetDrawColor(255, 0, 0, 200);
    }

    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.FillRect(dst);
}

double SDLManager::calculate_angle_to_checkpoint(const CarPositionDTO& player,
                                                 const CheckpointDTO& checkpoint) {
    
    const int dx = checkpoint.x - player.x_position;
    const int dy = checkpoint.y - player.y_position;

    const double angle_rad = std::atan2(dy, dx);
    double angle_deg = angle_rad * (180.0 / M_PI);
    
    
    // Convertir para que 0° = arriba
    angle_deg += 90.0;
    
    // Normalizar
    if (angle_deg < 0) angle_deg += 360.0;
    if (angle_deg >= 360.0) angle_deg -= 360.0;
    
    
    return angle_deg;
}

void SDLManager::render_hint(const CarPositionDTO& player) {
    double angle = calculate_angle_to_checkpoint(player, player.next);

    SDL2pp::Rect view = camera.get_view_rect();

    const int car_screen_x = (player.x_position - view.x) * 1920 / view.w;
    const int car_screen_y = (player.y_position - view.y) * 1080 / view.h;

    const int arrow_w = 60;
    const int arrow_h = 60;

    SDL2pp::Rect dst(car_screen_x - arrow_w / 2, car_screen_y - 140, arrow_w, arrow_h);

    SDL2pp::Texture& arrow = resources.get_texture("client/assets/sprites/arrow.png");
    renderer.Copy(arrow, SDL2pp::NullOpt, dst, angle, SDL2pp::NullOpt, SDL_FLIP_NONE);
}

void SDLManager::load_city(const std::string& city_name) {
    // todo en minusculas

    map_name = "client/assets/sprites/Game Boy _ GBC - Grand Theft Auto - Backgrounds - " +
               city_name + ".png";

    std::string city_name_lower = city_name;
    std::transform(city_name_lower.begin(), city_name_lower.end(), city_name_lower.begin(),
                   ::tolower);
    // separate city_name lower with _
    std::replace(city_name_lower.begin(), city_name_lower.end(), ' ', '_');
    decoration = "server/world/decorations_" + city_name_lower + ".json";
    bridges = "server/world/bridges_" + city_name_lower + ".json";

    set_minimap();
    load_bridges_rects(load_json_file(bridges));
    load_decorations_rects(load_json_file(decoration));
}
