#include "sdl_manager.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>

#include "../common/leaderboard_dto.h"

#include "client_constants.h"
#include "game_state.h"
#include "leaderboard_info.h"

SDLManager::SDLManager():
        sdl(SDL_INIT_VIDEO),
        mix(MIX_INIT_OGG),
        mixer(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024),
        window("NFS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
               SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        resources(renderer),
        camera(CAMERA_VIEW_WIDTH, CAMERA_VIEW_HEIGHT, WORLD_WIDTH, WORLD_HEIGHT),
        sounds(mixer, resources),
        animations(renderer, camera, resources),
        shop(renderer, mixer, resources, sounds, SCREEN_WIDTH, SCREEN_HEIGHT, FONT_PATH),
        leaderboard_info(renderer, resources, SCREEN_WIDTH, SCREEN_HEIGHT, FONT_PATH) {
    renderer.SetLogicalSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    Mix_MasterVolume(MIX_MAX_VOLUME * GAME_VOLUME_PERCENT / 100);

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dis(0, WAITING_ROOMS.size() - 1);
    const std::string& selected = WAITING_ROOMS[dis(gen)];

    waiting_room_background = &resources.get_texture(selected);
}

void SDLManager::set_minimap() {
    minimap = SDL2pp::Texture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                              MINIMAP_WIDTH, MINIMAP_HEIGHT);
    renderer.SetTarget(minimap.value());
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

nlohmann::json SDLManager::parse_file_data(const std::string& json_data) {

    try {

        if (json_data.empty())
            return nlohmann::json::array();

        return nlohmann::json::parse(json_data);
    } catch (const std::exception& e) {

        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return nlohmann::json::array();
    }
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

SDL2pp::Rect SDLManager::get_car_sprite_rect(const std::string& model) {
    auto it = CAR_SPRITES.find(model);
    if (it != CAR_SPRITES.end()) {
        return it->second;
    }
    return SDL2pp::Rect(DEFAULT_SPRITE_X, DEFAULT_SPRITE_Y, DEFAULT_SPRITE_W, DEFAULT_SPRITE_H);
}

void SDLManager::render_player(const CarPositionDTO& player, const std::string& model,
                               bool is_moving, const SDL2pp::Rect& view_rect, double scale_x,
                               double scale_y, SDL2pp::Texture& cars_texture) {
    const int car_screen_x = static_cast<int>((player.x_position - view_rect.x) * scale_x);
    const int car_screen_y = static_cast<int>((player.y_position - view_rect.y) * scale_y);
    const SDL2pp::Rect sprite_rect = get_car_sprite_rect(model);
    // Usar solo sprite vertical y rotarlo con SDL
    renderer.Copy(cars_texture, sprite_rect,
                  SDL2pp::Rect(car_screen_x - CAR_RENDER_OFFSET, car_screen_y - CAR_RENDER_OFFSET,
                               CAR_RENDER_SIZE, CAR_RENDER_SIZE),
                  static_cast<double>(player.angle), SDL2pp::NullOpt, SDL_FLIP_NONE);

    sounds.play_engine_sound(is_moving);
}

void SDLManager::render_npc(const NpcDTO& npc, const SDL2pp::Rect& view_rect, double scale_x,
                            double scale_y, SDL2pp::Texture& cars_texture) {
    const int car_screen_x = static_cast<int>((npc.x_position - view_rect.x) * scale_x);
    const int car_screen_y = static_cast<int>((npc.y_position - view_rect.y) * scale_y);

    // Use a default NPC car sprite (Compact car)
    SDL2pp::Rect sprite_rect =
            SDL2pp::Rect(NPC_SPRITE_X, NPC_SPRITE_Y, NPC_SPRITE_SIZE, NPC_SPRITE_SIZE);

    renderer.Copy(cars_texture, sprite_rect,
                  SDL2pp::Rect(car_screen_x - CAR_RENDER_OFFSET, car_screen_y - CAR_RENDER_OFFSET,
                               CAR_RENDER_SIZE, CAR_RENDER_SIZE),
                  static_cast<double>(npc.angle + NPC_ANGLE_OFFSET), SDL2pp::NullOpt,
                  SDL_FLIP_NONE);
}

// The power-up PNGs are transparent: they are copied as they are, with no background
void SDLManager::render_power_ups(const LocalInfo& game_state, const PowerUpDrawPass pass,
                                  const SDL2pp::Rect& view_rect, double scale_x, double scale_y) {
    // Size in the world of a power-up drawn POWER_UP_RENDER_SIZE wide, to know if it is on screen
    const int world_size = static_cast<int>(POWER_UP_RENDER_SIZE / scale_x);

    for (const auto& power_up: power_ups_to_draw(game_state, pass)) {

        SDL2pp::Rect world_rect(power_up.x - world_size / 2, power_up.y - world_size / 2,
                                world_size, world_size);
        if (!camera.on_screen(world_rect))
            continue;

        const int screen_x = static_cast<int>((power_up.x - view_rect.x) * scale_x);
        const int screen_y = static_cast<int>((power_up.y - view_rect.y) * scale_y);
        renderer.Copy(resources.get_texture(power_up.sprite), SDL2pp::NullOpt,
                      SDL2pp::Rect(screen_x - power_up.size / 2, screen_y - power_up.size / 2,
                                   power_up.size, power_up.size));
    }
}

void SDLManager::render_hud_effects(const LocalInfo& game_state) {
    const SDL2pp::Color text_color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);

    for (const auto& badge: effect_badges(game_state)) {

        renderer.Copy(resources.get_texture(badge.sprite), SDL2pp::NullOpt,
                      SDL2pp::Rect(badge.x, badge.y, POWER_UP_RENDER_SIZE, POWER_UP_RENDER_SIZE));

        // The seconds go next to the sprite, scaled down if they do not fit in the badge
        SDL2pp::Texture& text =
                resources.get_text(badge.text, FONT_PATH, EFFECT_BADGE_FONT_SIZE, text_color);
        const int text_w = std::min(text.GetWidth(), EFFECT_BADGE_TEXT_WIDTH);
        const int text_h = text.GetHeight() * text_w / std::max(text.GetWidth(), 1);
        renderer.Copy(text, SDL2pp::NullOpt,
                      SDL2pp::Rect(badge.x + POWER_UP_RENDER_SIZE, badge.y + (badge.h - text_h) / 2,
                                   text_w, text_h));
    }
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

void SDLManager::render_hud_minimap(int player_x, int player_y,
                                    const CheckpointDTO& next_checkpoint) {
    const int minimap_x = SCREEN_WIDTH - MINIMAP_WIDTH - MINIMAP_MARGIN;
    const int minimap_y = SCREEN_HEIGHT - MINIMAP_HEIGHT - MINIMAP_MARGIN;

    // Borde del minimapa
    renderer.SetDrawColor(COLOR_BLACK_R, COLOR_BLACK_G, COLOR_BLACK_B, COLOR_BLACK_A);
    renderer.FillRect(SDL2pp::Rect(minimap_x - MINIMAP_BORDER_THICKNESS,
                                   minimap_y - MINIMAP_BORDER_THICKNESS,
                                   MINIMAP_WIDTH + 2 * MINIMAP_BORDER_THICKNESS,
                                   MINIMAP_HEIGHT + 2 * MINIMAP_BORDER_THICKNESS));

    // Dibujar minimapa
    renderer.Copy(minimap.value(), SDL2pp::NullOpt,
                  SDL2pp::Rect(minimap_x, minimap_y, MINIMAP_WIDTH, MINIMAP_HEIGHT));

    // Jugador en el minimapa
    const int player_minimap_x = minimap_x + (player_x * MINIMAP_WIDTH / WORLD_WIDTH);
    const int player_minimap_y = minimap_y + (player_y * MINIMAP_HEIGHT / WORLD_HEIGHT);

    // Dibujar jugador en minimapa
    renderer.SetDrawColor(COLOR_YELLOW_R, COLOR_YELLOW_G, COLOR_YELLOW_B, COLOR_YELLOW_A);
    renderer.FillRect(SDL2pp::Rect(player_minimap_x - MINIMAP_PLAYER_OFFSET,
                                   player_minimap_y - MINIMAP_PLAYER_OFFSET, MINIMAP_PLAYER_SIZE,
                                   MINIMAP_PLAYER_SIZE));

    // Checkpoint en el minimapa
    const int checkpoint_minimap_x = minimap_x + (next_checkpoint.x * MINIMAP_WIDTH / WORLD_WIDTH);
    const int checkpoint_minimap_y =
            minimap_y + (next_checkpoint.y * MINIMAP_HEIGHT / WORLD_HEIGHT);

    // Dibujar checkpoint en minimapa (rojo si es normal, blanco si es la meta)
    if (next_checkpoint.is_finish_line) {
        renderer.SetDrawColor(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);
    } else {
        renderer.SetDrawColor(COLOR_RED_R, COLOR_RED_G, COLOR_RED_B, COLOR_RED_A);
    }
    renderer.FillRect(SDL2pp::Rect(checkpoint_minimap_x - MINIMAP_PLAYER_OFFSET,
                                   checkpoint_minimap_y - MINIMAP_PLAYER_OFFSET,
                                   MINIMAP_PLAYER_SIZE, MINIMAP_PLAYER_SIZE));
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

void SDLManager::render_players(double scale_x, double scale_y, const SDL2pp::Rect& view_rect,
                                const CarPositionDTO& my_car, const PlayerInfo& my_player,
                                const LocalInfo& game_state, SDL2pp::Texture& cars_texture,
                                SDL2pp::Texture& map, SDL2pp::Texture& arrow_texture) {
    // Para calcular si el auto esta fuera de la vista del jugador
    const int car_world_size = static_cast<int>(CAR_RENDER_SIZE / scale_x);
    const int car_world_half = car_world_size / 2;

    // Render NPCs on ground layer (si estan en pantalla)
    for (const auto& npc: game_state.npcs) {
        SDL2pp::Rect npc_rect(npc.x_position - car_world_half, npc.y_position - car_world_half,
                              car_world_size, car_world_size);
        if (camera.on_screen(npc_rect)) {
            render_npc(npc, view_rect, scale_x, scale_y, cars_texture);
        }
    }

    // Los power-ups quedan debajo de los autos; los de abajo y los del suelo, debajo de los puentes
    render_power_ups(game_state, PowerUpDrawPass::BELOW_BRIDGES, view_rect, scale_x, scale_y);

    // Renderizar autos desde el suelo
    if (my_car.layer != CarLayer::ON_BRIDGE) {
        render_player(my_car, my_player.model, my_player.isMoving, view_rect, scale_x, scale_y,
                      cars_texture);
    }

    // Renderizar otros jugadores en suelo (si estan en pantalla)
    for (const auto& [pid, pinfo]: game_state.players) {
        if (pinfo.layer != CarLayer::ON_BRIDGE) {
            SDL2pp::Rect player_rect(pinfo.x - car_world_half, pinfo.y - car_world_half,
                                     car_world_size, car_world_size);
            if (camera.on_screen(player_rect)) {
                CarPositionDTO pos;
                pos.x_position = pinfo.x;
                pos.y_position = pinfo.y;
                pos.angle = pinfo.angle;
                pos.layer = pinfo.layer;
                pos.next = pinfo.checkpoint;
                render_player(pos, pinfo.model, pinfo.isMoving, view_rect, scale_x, scale_y,
                              cars_texture);
            }
        }
    }

    render_bridges(view_rect, scale_x, scale_y, map);

    // Los power-ups sobre un puente, encima del puente y debajo de los autos de arriba
    render_power_ups(game_state, PowerUpDrawPass::ABOVE_BRIDGES, view_rect, scale_x, scale_y);

    // Renderizar jugadores sobre puentes
    if (my_car.layer == CarLayer::ON_BRIDGE) {
        render_player(my_car, my_player.model, my_player.isMoving, view_rect, scale_x, scale_y,
                      cars_texture);
    }

    // Renderizar otros jugadores en puentes (si estan en pantalla)
    for (const auto& [pid, pinfo]: game_state.players) {
        if (pinfo.layer == CarLayer::ON_BRIDGE) {
            SDL2pp::Rect player_rect(pinfo.x - car_world_half, pinfo.y - car_world_half,
                                     car_world_size, car_world_size);
            if (camera.on_screen(player_rect)) {
                CarPositionDTO pos;
                pos.x_position = pinfo.x;
                pos.y_position = pinfo.y;
                pos.angle = pinfo.angle;
                pos.layer = pinfo.layer;
                pos.next = pinfo.checkpoint;
                render_player(pos, pinfo.model, pinfo.isMoving, view_rect, scale_x, scale_y,
                              cars_texture);
            }
        }
    }

    render_decorations(view_rect, scale_x, scale_y, map);
    render_checkpoints(my_car.next, view_rect, scale_x, scale_y);
    render_hint(my_car, view_rect, scale_x, scale_y, arrow_texture);
    render_stats(my_player);
}

void SDLManager::set_sound_info(const std::vector<uint32_t>& ids) {
    sounds.initialize_channels(ids);
}

void SDLManager::play_background_music(const int loops) { sounds.play_background_music(loops); }

void SDLManager::stop_background_music() { sounds.stop_background_music(); }

void SDLManager::set_music_volume(const int volume) { sounds.set_music_volume(volume); }

void SDLManager::render(LocalInfo& game_state) {
    SDL_BlendMode prev_blend = renderer.GetDrawBlendMode();
    SDL_Color prev_color = renderer.GetDrawColor();

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

    // Usar flag died para detectar muerte
    bool player_is_dead = game_state.player.died || game_state.player.life <= 0;
    if (player_is_dead && !game_state.player.explosion_shown) {
        animations.start_explosion(game_state.id, iteration_counter, SDL2pp::Point(pos_x, pos_y));
        animations.start_camera_shake(iteration_counter);
        SDL2pp::Point player_pos(pos_x, pos_y);
        sounds.play_explosion_sound(game_state.id, player_pos, player_pos);
        game_state.player.explosion_shown = true;
    }

    if (!player_is_dead) {
        game_state.player.explosion_shown = false;
    }

    SDL2pp::Point target_pos = camera.get_center();

    // Usar flag crashed para detectar crash del cliente
    if (game_state.player.crashed) {
        if (!animations.is_camera_shake_active(iteration_counter)) {
            animations.start_camera_shake(iteration_counter);
            SDL2pp::Point player_pos(pos_x, pos_y);
            sounds.play_crash_sound(game_state.id, player_pos, player_pos);
        }
        game_state.player.crashed = false;
    }

    // Unica iteracion para detectar explosiones y crashes de otros jugadores
    for (auto& [pid, pinfo]: game_state.players) {
        // Detectar muerte/explosion
        bool is_dead = pinfo.died || pinfo.life <= 0;
        if (is_dead && !pinfo.explosion_shown) {
            animations.start_explosion(pid, iteration_counter, SDL2pp::Point(pinfo.x, pinfo.y));
            sounds.play_explosion_sound(pid, SDL2pp::Point{pinfo.x, pinfo.y}, target_pos);
            pinfo.explosion_shown = true;
        }
        if (!is_dead) {
            pinfo.explosion_shown = false;
        }

        // Detectar crash
        if (pinfo.crashed) {
            sounds.play_crash_sound(pid, SDL2pp::Point{pinfo.x, pinfo.y}, target_pos);
            pinfo.crashed = false;
        }
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

    render_players(scale_x, scale_y, view_rect, my_car, my_player, game_state, cars_texture, map,
                   arrow_texture);

    render_hud_health(game_state.player.life, game_state.player.max_life);
    render_hud_effects(game_state);
    animations.render_explosions(iteration_counter);
    animations.clean_explosions(iteration_counter);
    render_hud_minimap(pos_x, pos_y, my_player.checkpoint);


    if (my_player.died || my_player.life <= 0) {
        render_center_box("Your car exploded !", SDL2pp::Color(255, 0, 0, 255));
    } else if (my_player.finish_race) {
        render_center_box("You have finished the race !", SDL2pp::Color(255, 215, 0, 255));
    }

    renderer.SetDrawBlendMode(prev_blend);
    renderer.SetDrawColor(prev_color);

    show_screen();
}

void SDLManager::render_waiting() {
    clear_screen();

    renderer.Copy(*waiting_room_background, SDL2pp::NullOpt, SDL2pp::NullOpt);

    const std::string text = "Waiting for players...";
    SDL2pp::Color text_color(255, 255, 255, 255);  // blanco

    render_text(text, text_color, TEXT_WAITING_SIZE);

    show_screen();
}


void SDLManager::render_countdown(const LocalInfo& game_state) {
    SDL_BlendMode prev_blend = renderer.GetDrawBlendMode();
    SDL_Color prev_color = renderer.GetDrawColor();

    const PlayerInfo& my_player = game_state.player;
    CarPositionDTO my_car;
    my_car.x_position = my_player.x;
    my_car.y_position = my_player.y;
    my_car.angle = my_player.angle;
    my_car.layer = my_player.layer;
    my_car.next = my_player.checkpoint;

    const uint32_t pos_x = my_car.x_position;
    const uint32_t pos_y = my_car.y_position;

    update_camera(pos_x, pos_y);
    clear_screen();

    // render map in the background
    SDL2pp::Rect view_rect = get_view_rect_with_shake();
    SDL2pp::Texture& map = resources.get_texture(map_name);

    renderer.Copy(map, view_rect, SDL2pp::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
    SDL2pp::Texture& cars_texture = resources.get_texture(CARS_SPRITE_PATH);
    SDL2pp::Texture& arrow_texture = resources.get_texture(ARROW_SPRITE_PATH);

    const double scale_x = static_cast<double>(SCREEN_WIDTH) / view_rect.w;
    const double scale_y = static_cast<double>(SCREEN_HEIGHT) / view_rect.h;

    render_players(scale_x, scale_y, view_rect, my_car, my_player, game_state, cars_texture, map,
                   arrow_texture);


    const std::string text = "The race will start in " +
                             std::to_string(game_state.countdown_seconds + 1) + " seconds";
    SDL2pp::Color text_color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);
    render_text(text, text_color, TEXT_COUNTDOWN_SIZE);

    renderer.SetDrawBlendMode(prev_blend);
    renderer.SetDrawColor(prev_color);
    show_screen();
}

void SDLManager::render_leaderboard(const LeaderboardDTO& leaderboard, const std::string& title) {
    leaderboard_info.render(leaderboard, title, map_name, camera);
}

void SDLManager::render_center_box(const std::string& message, const SDL2pp::Color& color) {

    SDL_BlendMode prev_blend = renderer.GetDrawBlendMode();
    SDL_Color prev_color = renderer.GetDrawColor();

    int rect_w = SCREEN_WIDTH * 1 / 2;
    int rect_h = SCREEN_HEIGHT * 1 / 6;
    int rect_x = (SCREEN_WIDTH - rect_w) / 2;
    int rect_y = (SCREEN_HEIGHT - rect_h) / 2;

    SDL2pp::Rect board_rect(rect_x, rect_y, rect_w, rect_h);

    SDL2pp::Color bg_color(30, 30, 30, 200);
    int border = 5;

    // Fondo transparente
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(bg_color);
    renderer.FillRect(board_rect);

    // Bordes
    renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
    renderer.SetDrawColor(color);
    renderer.FillRect(SDL2pp::Rect(rect_x, rect_y, rect_w, border));                    // top
    renderer.FillRect(SDL2pp::Rect(rect_x, rect_y + rect_h - border, rect_w, border));  // bottom
    renderer.FillRect(SDL2pp::Rect(rect_x, rect_y, border, rect_h));                    // left
    renderer.FillRect(SDL2pp::Rect(rect_x + rect_w - border, rect_y, border, rect_h));  // right

    // Texto
    SDL2pp::Texture& msg_texture =
            resources.get_text(message, FONT_PATH, LEADERBOARD_HEADER_FONT_SIZE, color);

    int msg_x = rect_x + (rect_w - msg_texture.GetWidth()) / 2;
    int msg_y = rect_y + (rect_h - msg_texture.GetHeight()) / 2;

    renderer.Copy(msg_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(msg_x, msg_y, msg_texture.GetWidth(), msg_texture.GetHeight()));

    // Restaurar estado
    renderer.SetDrawBlendMode(prev_blend);
    renderer.SetDrawColor(prev_color);
}

void SDLManager::draw_row(const std::string& sentence, int x, int y, const SDL2pp::Color& color,
                          int font_size) {

    SDL2pp::Texture& text_texture = resources.get_text(sentence, FONT_PATH, font_size, color);

    renderer.Copy(text_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(x, y, text_texture.GetWidth(), text_texture.GetHeight()));
}

void SDLManager::render_stats(const PlayerInfo& player) {
    int box_w = SCREEN_WIDTH * STATS_BOX_WIDTH_RATIO / STATS_BOX_DIVISOR;
    int box_h = SCREEN_HEIGHT * STATS_BOX_HEIGHT_RATIO / STATS_BOX_DIVISOR;
    int box_x = (SCREEN_WIDTH - box_w) / 2;
    int box_y = (SCREEN_HEIGHT - box_h) / 2;

    SDL2pp::Rect stats{box_x, box_y, box_w, box_h};

    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(COLOR_BLACK_R, COLOR_BLACK_G, COLOR_BLACK_B, 0);
    renderer.DrawRect(stats);

    SDL2pp::Color text_color =
            SDL2pp::Color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);

    int text_x = box_x + STATS_MARGIN;
    int text_y = box_y + STATS_MARGIN;
    int font_size = box_h / STATS_FONT_SIZE_DIVISOR;
    int spacing = font_size + STATS_SPACING_OFFSET;
    int o = 2;                                  // grosor del borde
    SDL2pp::Color outline_color(0, 0, 0, 255);  // negro

    std::string player_name = "Player :  " + player.username;

    draw_row(player_name, text_x - o, text_y - o, outline_color, font_size);
    draw_row(player_name, text_x + o, text_y - o, outline_color, font_size);
    draw_row(player_name, text_x - o, text_y + o, outline_color, font_size);
    draw_row(player_name, text_x + o, text_y + o, outline_color, font_size);
    draw_row(player_name, text_x, text_y, text_color, font_size);

    text_y += spacing;

    std::string player_time = "Time :  " + std::to_string(player.time);
    draw_row(player_time, text_x - o, text_y - o, outline_color, font_size);
    draw_row(player_time, text_x + o, text_y - o, outline_color, font_size);
    draw_row(player_time, text_x - o, text_y + o, outline_color, font_size);
    draw_row(player_time, text_x + o, text_y + o, outline_color, font_size);
    draw_row(player_time, text_x, text_y, text_color, font_size);
    text_y += spacing;
    std::string player_lap = "Lap :  " + std::to_string(player.current_lap) + " of " +
                             std::to_string(player.race_lap);

    draw_row(player_lap, text_x - o, text_y - o, outline_color, font_size);
    draw_row(player_lap, text_x + o, text_y - o, outline_color, font_size);
    draw_row(player_lap, text_x - o, text_y + o, outline_color, font_size);
    draw_row(player_lap, text_x + o, text_y + o, outline_color, font_size);
    draw_row(player_lap, text_x, text_y, text_color, font_size);
}

void SDLManager::load_bridges_rects(const nlohmann::json& bridges_rects_data) {
    bridges_rects = loadRects(bridges_rects_data);
}

void SDLManager::load_decorations_rects(const nlohmann::json& decorations_rects_data) {
    decorations_rects = loadRects(decorations_rects_data);
}

std::optional<ButtonType> SDLManager::interact_button(int x, int y, int time) {
    return shop.interact_button(x, y, time);
}

void SDLManager::render_shop(int time, int time_penalty, int upgrades_remaining,
                             int upgrade_shop_countdown) {
    shop.render(time, Car::DEFAULT, map_name, camera, time_penalty, upgrades_remaining,
                upgrade_shop_countdown);
}

void SDLManager::close_shop() { shop.close_shop(); }

bool SDLManager::is_shop_open() { return shop.is_shop_open(); }

void SDLManager::toggle_shop() { shop.toggle_shop(); }

void SDLManager::close_leaderboard() { leaderboard_info.close_leaderboard(); }
void SDLManager::open_leaderboard() { leaderboard_info.open_leaderboard(); }
bool SDLManager::is_leaderboard_open() { return leaderboard_info.is_leaderboard_open(); }

void SDLManager::play_upgrade_sound() { sounds.play_upgrade_sound(); }

void SDLManager::play_finished_race_sound() { sounds.play_finished_race_sound(); }

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
        renderer.SetDrawColor(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_RED_ALPHA);
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

    SDL2pp::Rect dst(car_screen_x - ARROW_WIDTH / 2, car_screen_y - ARROW_VERTICAL_OFFSET,
                     ARROW_WIDTH, ARROW_HEIGHT);

    renderer.Copy(arrow_texture, SDL2pp::NullOpt, dst, angle, SDL2pp::NullOpt, SDL_FLIP_NONE);
}

void SDLManager::load_city(const std::string& city_name, const std::string& bridges_data,
                           const std::string& decorations_data) {

    map_name = std::string(MAP_BACKGROUND_PREFIX) + city_name + MAP_BACKGROUND_SUFFIX;

    nlohmann::json parsed_bridges_data = parse_file_data(bridges_data);
    nlohmann::json parsed_decorations_data = parse_file_data(decorations_data);

    set_minimap();
    load_bridges_rects(parsed_bridges_data);
    load_decorations_rects(parsed_decorations_data);
}

void SDLManager::render_text(const std::string& text, const SDL2pp::Color& text_color, int size) {
    SDL2pp::Texture& text_texture = resources.get_text(text, FONT_PATH, size, text_color);
    int text_x = (SCREEN_WIDTH - text_texture.GetWidth()) / 2;
    int text_y = (SCREEN_HEIGHT - text_texture.GetHeight()) / 2;

    int o = 2;                                  // grosor del borde
    SDL2pp::Color outline_color(0, 0, 0, 255);  // negro

    draw_row(text, text_x - o, text_y - o, outline_color, size);
    draw_row(text, text_x + o, text_y - o, outline_color, size);
    draw_row(text, text_x - o, text_y + o, outline_color, size);
    draw_row(text, text_x + o, text_y + o, outline_color, size);

    draw_row(text, text_x - o, text_y, outline_color, size);
    draw_row(text, text_x + o, text_y, outline_color, size);
    draw_row(text, text_x, text_y - o, outline_color, size);
    draw_row(text, text_x, text_y + o, outline_color, size);

    draw_row(text, text_x, text_y, text_color, size);
}
