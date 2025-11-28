#include "sdl_manager.h"

#include <algorithm>
#include <fstream>
#include <map>

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <qforeach.h>

#include "client_constants.h"
#include "game_state.h"

SDLManager::SDLManager():
        sdl(SDL_INIT_VIDEO),
        mix(MIX_INIT_OGG),
        mixer(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024),
        window("NFS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
               SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        resources(renderer),
        camera(CAMERA_VIEW_WIDTH, CAMERA_VIEW_HEIGHT, WORLD_WIDTH, WORLD_HEIGHT),
        sounds(mixer, resources),
        animations(renderer, camera, resources),
        shop(renderer, mixer, resources, sounds, SCREEN_WIDTH, SCREEN_HEIGHT, FONT_PATH) {
    renderer.SetLogicalSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    load_city(DEFAULT_CITY);
}

void SDLManager::set_minimap() {
    minimap = SDL2pp::Texture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                              MINIMAP_WIDTH, MINIMAP_HEIGHT);
    renderer.SetTarget(minimap.value());
    std::cout << "Cargando minimap de " << map_name << std::endl;
    SDL2pp::Texture& map = resources.get_texture(map_name);
    renderer.Copy(map, SDL2pp::Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT),
                  SDL2pp::Rect(0, 0, MINIMAP_WIDTH, MINIMAP_HEIGHT));
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

SDL2pp::Rect SDLManager::get_view_rect_with_shake() {
    SDL2pp::Rect view_rect = camera.get_view_rect();
    // Aplicar camera shake si está activo
    if (animations.is_camera_shake_active(iteration_counter)) {
        SDL2pp::Point shake_offset = animations.get_camera_shake();
        view_rect.x += shake_offset.GetX();
        view_rect.y += shake_offset.GetY();
    }
    return view_rect;
}

void SDLManager::render_player(const CarPositionDTO& player, const std::string& model,
                               bool is_moving, const SDL2pp::Rect& view_rect, double scale_x,
                               double scale_y, SDL2pp::Texture& cars_texture) {
    const int car_screen_x = static_cast<int>((player.x_position - view_rect.x) * scale_x);
    const int car_screen_y = static_cast<int>((player.y_position - view_rect.y) * scale_y);
    SDL2pp::Rect sprite_rect = SDL2pp::Rect(DEFAULT_SPRITE_X, DEFAULT_SPRITE_Y, DEFAULT_SPRITE_W, DEFAULT_SPRITE_H);

    auto it = CAR_SPRITES.find(model);
    if (it != CAR_SPRITES.end()) {
        sprite_rect = it->second;
    }

    // Usar solo sprite vertical y rotarlo con SDL
    renderer.Copy(cars_texture, sprite_rect, SDL2pp::Rect(car_screen_x - CAR_RENDER_OFFSET, car_screen_y - CAR_RENDER_OFFSET, CAR_RENDER_SIZE, CAR_RENDER_SIZE),
                  static_cast<double>(player.angle), SDL2pp::NullOpt, SDL_FLIP_NONE);

    sounds.play_engine_sound(is_moving);
}

void SDLManager::render_npc(const NpcDTO& npc, const SDL2pp::Rect& view_rect, double scale_x,
                             double scale_y, SDL2pp::Texture& cars_texture) {
    const int car_screen_x = static_cast<int>((npc.x_position - view_rect.x) * scale_x);
    const int car_screen_y = static_cast<int>((npc.y_position - view_rect.y) * scale_y);

    // Use a default NPC car sprite (Compact car)
    SDL2pp::Rect sprite_rect = SDL2pp::Rect(NPC_SPRITE_X, NPC_SPRITE_Y, NPC_SPRITE_SIZE, NPC_SPRITE_SIZE);

    renderer.Copy(cars_texture, sprite_rect, SDL2pp::Rect(car_screen_x - CAR_RENDER_OFFSET, car_screen_y - CAR_RENDER_OFFSET, CAR_RENDER_SIZE, CAR_RENDER_SIZE),
                  static_cast<double>(npc.angle + NPC_ANGLE_OFFSET), SDL2pp::NullOpt, SDL_FLIP_NONE);
}

void SDLManager::render_hud_health(const int health, const int max_health) {
    // Fondo de la barra
    renderer.SetDrawColor(COLOR_BLACK_R, COLOR_BLACK_G, COLOR_BLACK_B, COLOR_BLACK_A);
    renderer.FillRect(
            SDL2pp::Rect(HEALTH_BAR_X, HEALTH_BAR_Y, HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));

    // Dibujar barra de vida segun vida maxima del auto
    int current_health_width = 0;
    if (max_health > 0) {
        current_health_width = (health * HEALTH_BAR_WIDTH) / max_health;
        if (current_health_width > HEALTH_BAR_WIDTH) {
            current_health_width = HEALTH_BAR_WIDTH;
        }
    }
    renderer.SetDrawColor(COLOR_RED_R, COLOR_RED_G, COLOR_RED_B, COLOR_RED_A);
    renderer.FillRect(
            SDL2pp::Rect(HEALTH_BAR_X, HEALTH_BAR_Y, current_health_width, HEALTH_BAR_HEIGHT));
}

void SDLManager::render_hud_minimap(int player_x, int player_y) {
    const int minimap_x = SCREEN_WIDTH - MINIMAP_WIDTH - MINIMAP_MARGIN;
    const int minimap_y = SCREEN_HEIGHT - MINIMAP_HEIGHT - MINIMAP_MARGIN;

    // Borde del minimapa
    renderer.SetDrawColor(COLOR_BLACK_R, COLOR_BLACK_G, COLOR_BLACK_B, COLOR_BLACK_A);
    renderer.FillRect(
            SDL2pp::Rect(minimap_x - MINIMAP_BORDER_THICKNESS, minimap_y - MINIMAP_BORDER_THICKNESS,
                         MINIMAP_WIDTH + 2 * MINIMAP_BORDER_THICKNESS, MINIMAP_HEIGHT + 2 * MINIMAP_BORDER_THICKNESS));

    // Dibujar minimapa
    renderer.Copy(minimap.value(), SDL2pp::NullOpt,
                  SDL2pp::Rect(minimap_x, minimap_y, MINIMAP_WIDTH, MINIMAP_HEIGHT));

    // Jugador en el minimapa
    const int player_minimap_x = minimap_x + (player_x * MINIMAP_WIDTH / WORLD_WIDTH);
    const int player_minimap_y = minimap_y + (player_y * MINIMAP_HEIGHT / WORLD_HEIGHT);

    // Dibujar jugador en minimapa
    renderer.SetDrawColor(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B, COLOR_YELLOW_A);
    renderer.FillRect(SDL2pp::Rect(player_minimap_x - MINIMAP_PLAYER_OFFSET, player_minimap_y - MINIMAP_PLAYER_OFFSET, MINIMAP_PLAYER_SIZE, MINIMAP_PLAYER_SIZE));
}

void SDLManager::render_layer(const std::vector<SDL2pp::Rect>& rects, const SDL2pp::Rect& view,
                              double scale_x, double scale_y, SDL2pp::Texture& map_texture) {
    for (const auto& src_rect: rects) {
        if (!camera.on_screen(src_rect))
            continue;

        // Vista en pantalla
        const int dx = static_cast<int>((src_rect.x - view.x) * scale_x);
        const int dy = static_cast<int>((src_rect.y - view.y) * scale_y);
        const int dw = static_cast<int>(src_rect.w * scale_x);
        const int dh = static_cast<int>(src_rect.h * scale_y);
        const SDL2pp::Rect dst_rect(dx, dy, dw, dh);

        renderer.Copy(map_texture, src_rect, dst_rect);
    }
}

void SDLManager::render_bridges(const SDL2pp::Rect& view_rect, double scale_x, double scale_y,
                                SDL2pp::Texture& map_texture) {
    render_layer(bridges_rects, view_rect, scale_x, scale_y, map_texture);
}

void SDLManager::render_decorations(const SDL2pp::Rect& view_rect, double scale_x, double scale_y,
                                    SDL2pp::Texture& map_texture) {
    render_layer(decorations_rects, view_rect, scale_x, scale_y, map_texture);
}

void SDLManager::render(LocalInfo& game_state) {
    iteration_counter++;

    // Uso directo de game_state.player
    const PlayerInfo& my_player = game_state.player;
    CarPositionDTO my_car;
    my_car.x_position = my_player.x;
    my_car.y_position = my_player.y;
    my_car.angle = my_player.angle;
    my_car.layer = my_player.layer;
    my_car.next = my_player.checkpoint;

    const uint32_t pos_x = my_car.x_position;
    const uint32_t pos_y = my_car.y_position;

    /* if (!animations.is_explosion_active(iteration_counter)) {
        animations.start_explosion(iteration_counter,
                                   SDL2pp::Point(pos_x, pos_y));
    } */

    if (game_state.player.life <= 0 && !game_state.player.died) {
        game_state.player.died = true;
        animations.start_camera_shake(iteration_counter);
    }

    update_camera(pos_x, pos_y);
    clear_screen();

    // Calcular view_rect y escala una sola vez
    SDL2pp::Rect view_rect = get_view_rect_with_shake();
    const double scale_x = static_cast<double>(SCREEN_WIDTH) / view_rect.w;
    const double scale_y = static_cast<double>(SCREEN_HEIGHT) / view_rect.h;

    // Solo una busqueda en hash map
    SDL2pp::Texture& map = resources.get_texture(map_name);
    SDL2pp::Texture& cars_texture = resources.get_texture(CARS_SPRITE_PATH);
    SDL2pp::Texture& arrow_texture = resources.get_texture(ARROW_SPRITE_PATH);

    renderer.Copy(map, view_rect, SDL2pp::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

    // Render NPCs on ground layer
    for (const auto& npc: game_state.npcs) {
        render_npc(npc, view_rect, scale_x, scale_y, cars_texture);
    }

    // Renderizar autos desde el suelo
    if (my_car.layer != CarLayer::ON_BRIDGE) {
        render_player(my_car, my_player.model, true, view_rect, scale_x, scale_y, cars_texture);
    }

    for (const auto& [pid, pinfo]: game_state.players) {
        CarPositionDTO pos;
        pos.x_position = pinfo.x;
        pos.y_position = pinfo.y;
        pos.angle = pinfo.angle;
        pos.layer = pinfo.layer;
        pos.next = pinfo.checkpoint;

        if (pos.layer != CarLayer::ON_BRIDGE) {
            render_player(pos, pinfo.model, true, view_rect, scale_x, scale_y, cars_texture);
        }
    }

    render_bridges(view_rect, scale_x, scale_y, map);

    // Renderizar jugadores sobre puentes
    if (my_car.layer == CarLayer::ON_BRIDGE) {
        render_player(my_car, my_player.model, true, view_rect, scale_x, scale_y, cars_texture);
    }

    for (const auto& [pid, pinfo]: game_state.players) {
        CarPositionDTO pos;
        pos.x_position = pinfo.x;
        pos.y_position = pinfo.y;
        pos.angle = pinfo.angle;
        pos.layer = pinfo.layer;
        pos.next = pinfo.checkpoint;

        if (pos.layer == CarLayer::ON_BRIDGE) {
            render_player(pos, pinfo.model, true, view_rect, scale_x, scale_y, cars_texture);
        }
    }

    render_decorations(view_rect, scale_x, scale_y, map);

    render_checkpoints(my_car.next, view_rect, scale_x, scale_y);

    render_hud_health(game_state.player.life, game_state.player.max_life);
    render_hud_minimap(pos_x, pos_y);
    
    // Para renderizar las stats. TODO
    std::vector<std::pair<PlayerDTO, CarPositionDTO>> cars_for_stats;
    for (const auto& [pid, pinfo]: game_state.players) {
        PlayerDTO p;
        p.id = pid;
        p.car.model = pinfo.model;
        p.username = "";  // Sin campo username

        CarPositionDTO pos;
        pos.x_position = pinfo.x;
        pos.y_position = pinfo.y;
        pos.angle = pinfo.angle;
        pos.layer = pinfo.layer;
        pos.next = pinfo.checkpoint;

        cars_for_stats.emplace_back(p, pos);
    }
    render_stats(cars_for_stats);
    
    render_hint(my_car, view_rect, scale_x, scale_y, arrow_texture);

    // animations.render_explosion(iteration_counter);

    show_screen();
    SDL_Delay(RENDER_DELAY_MS);
}

void SDLManager::render_waiting() {
    clear_screen();
    const std::string text = "Waiting for players...";
    SDL2pp::Color text_color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);
    SDL2pp::Texture& text_texture = resources.get_text(text, FONT_PATH, TEXT_WAITING_SIZE, text_color);
    int text_x = (SCREEN_WIDTH - text_texture.GetWidth()) / 2;
    int text_y = (SCREEN_HEIGHT - text_texture.GetHeight()) / 2;
    draw_row(text, text_x, text_y, text_color, TEXT_WAITING_SIZE);
    show_screen();
}

void SDLManager::render_countdown() {
    clear_screen();
    const std::string text = "The race will start soon...";
    SDL2pp::Color text_color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);
    SDL2pp::Texture& text_texture = resources.get_text(text, FONT_PATH, TEXT_COUNTDOWN_SIZE, text_color);
    int text_x = (SCREEN_WIDTH - text_texture.GetWidth()) / 2;
    int text_y = (SCREEN_HEIGHT - text_texture.GetHeight()) / 2;
    draw_row(text, text_x, text_y, text_color, TEXT_COUNTDOWN_SIZE);
    show_screen();
}

void SDLManager::render_leaderboard() {
    clear_screen();
    const std::string text = "Leaderboard";
    SDL2pp::Color text_color(COLOR_GOLD_R, COLOR_GOLD_G, COLOR_GOLD_B, COLOR_GOLD_A);
    SDL2pp::Texture& text_texture = resources.get_text(text, FONT_PATH, TEXT_LEADERBOARD_SIZE, text_color);
    int text_x = (SCREEN_WIDTH - text_texture.GetWidth()) / 2;
    int text_y = (SCREEN_HEIGHT - text_texture.GetHeight()) / 2;
    draw_row(text, text_x, text_y, text_color, TEXT_LEADERBOARD_SIZE);
    show_screen();
}

void SDLManager::draw_row(const std::string& sentence, int x, int y, SDL2pp::Color color,
                          int font_size) {

    SDL2pp::Texture& text_texture =
            resources.get_text(sentence, FONT_PATH, font_size, color);

    renderer.Copy(text_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(x, y, text_texture.GetWidth(), text_texture.GetHeight()));
}

void SDLManager::render_stats(const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& cars) {
    int box_w = SCREEN_WIDTH * STATS_BOX_WIDTH_RATIO / STATS_BOX_DIVISOR;
    int box_h = SCREEN_HEIGHT * STATS_BOX_HEIGHT_RATIO / STATS_BOX_DIVISOR;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = (SCREEN_HEIGHT - box_h) / 2;

    SDL2pp::Rect stats{box_x, box_y, box_w, box_h};

    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(COLOR_BLACK_R, COLOR_BLACK_G, COLOR_BLACK_B, COLOR_BLACK_ALPHA);
    renderer.DrawRect(stats);

    SDL2pp::Color text_color = SDL2pp::Color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);

    int text_x = box_x + STATS_MARGIN;
    int text_y = box_y + STATS_MARGIN;
    int font_size = box_h / STATS_FONT_SIZE_DIVISOR;
    int spacing = font_size + STATS_SPACING_OFFSET;

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

bool SDLManager::is_shop_open() { return shop.is_shop_open(); }

void SDLManager::toggle_shop() { shop.toggle_shop(); }

void SDLManager::play_upgrade_sound() { sounds.play_upgrade_sound(); }

void SDLManager::render_checkpoints(const CheckpointDTO& checkpoint, const SDL2pp::Rect& view,
                                    double scale_x, double scale_y) {
    SDL2pp::Rect checkpoint_rect(checkpoint.x, checkpoint.y, checkpoint.width, checkpoint.height);
    if (!camera.on_screen(checkpoint_rect)) {
        return;
    }

    int screen_x = static_cast<int>(((checkpoint.x - checkpoint.width / 2) - view.x) * scale_x);
    int screen_y = static_cast<int>(((checkpoint.y - checkpoint.height / 2) - view.y) * scale_y);
    int screen_w = static_cast<int>(checkpoint.width * scale_x);
    int screen_h = static_cast<int>(checkpoint.height * scale_y);

    SDL2pp::Rect dst(screen_x, screen_y, screen_w, screen_h);

    if (checkpoint.is_finish_line) {
        renderer.SetDrawColor(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B, COLOR_YELLOW_ALPHA);
    } else {
        renderer.SetDrawColor(COLOR_RED_R, COLOR_RED_G, COLOR_RED_B, COLOR_RED_ALPHA);
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
    angle_deg += ANGLE_OFFSET_DEGREES;

    // Normalizar
    if (angle_deg < 0)
        angle_deg += FULL_CIRCLE_DEGREES;
    if (angle_deg >= FULL_CIRCLE_DEGREES)
        angle_deg -= FULL_CIRCLE_DEGREES;


    return angle_deg;
}

void SDLManager::render_hint(const CarPositionDTO& player, const SDL2pp::Rect& view, double scale_x,
                             double scale_y, SDL2pp::Texture& arrow_texture) {
    double angle = calculate_angle_to_checkpoint(player, player.next);

    const int car_screen_x = static_cast<int>((player.x_position - view.x) * scale_x);
    const int car_screen_y = static_cast<int>((player.y_position - view.y) * scale_y);

    SDL2pp::Rect dst(car_screen_x - ARROW_WIDTH / 2, car_screen_y - ARROW_VERTICAL_OFFSET, ARROW_WIDTH, ARROW_HEIGHT);

    renderer.Copy(arrow_texture, SDL2pp::NullOpt, dst, angle, SDL2pp::NullOpt, SDL_FLIP_NONE);
}

void SDLManager::load_city(const std::string& city_name) {
    // todo en minusculas

    map_name = std::string(MAP_BACKGROUND_PREFIX) + city_name + MAP_BACKGROUND_SUFFIX;

    std::string city_name_lower = city_name;
    std::transform(city_name_lower.begin(), city_name_lower.end(), city_name_lower.begin(),
                   ::tolower);
    // separate city_name lower with _
    std::replace(city_name_lower.begin(), city_name_lower.end(), ' ', '_');
    decoration = std::string(DECORATIONS_PREFIX) + city_name_lower + JSON_SUFFIX;
    bridges = std::string(BRIDGES_PREFIX) + city_name_lower + JSON_SUFFIX;

    set_minimap();
    load_bridges_rects(load_json_file(bridges));
    load_decorations_rects(load_json_file(decoration));
}
