#ifndef SDL_MANAGER_H
#define SDL_MANAGER_H
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "../common/leaderboard_dto.h"
#include "SDL2pp/Renderer.hh"
#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "common/car_position_dto.h"
#include "common/npc_dto.h"

#include "animations.h"
#include "camera.h"
#include "client_constants.h"
#include "game_state.h"
#include "leaderboard_info.h"
#include "resource_manager.h"
#include "sounds.h"
#include "upgrade_shop.h"

static const std::unordered_map<std::string, SDL2pp::Rect> CAR_SPRITES = {
        {"Compact", SDL2pp::Rect(0, 0, CAR_SPRITE_SIZE, CAR_SPRITE_SIZE)},
        {"Sport", SDL2pp::Rect(CAR_SPRITE_SIZE, 0, CAR_SPRITE_SIZE, CAR_SPRITE_SIZE)},
        {"Convertible", SDL2pp::Rect(2 * CAR_SPRITE_SIZE, 0, CAR_SPRITE_SIZE, CAR_SPRITE_SIZE)},
        {"Sedan", SDL2pp::Rect(3 * CAR_SPRITE_SIZE, 0, CAR_SPRITE_SIZE, CAR_SPRITE_SIZE)},
        {"Jeep", SDL2pp::Rect(4 * CAR_SPRITE_SIZE, 0, CAR_SPRITE_SIZE, CAR_SPRITE_SIZE)},
        {"Rally", SDL2pp::Rect(5 * CAR_SPRITE_SIZE, 0, CAR_SPRITE_SIZE, CAR_SPRITE_SIZE)},
        {"Truck", SDL2pp::Rect(6 * CAR_SPRITE_SIZE, 0, CAR_SPRITE_SIZE, CAR_SPRITE_SIZE)}};

class SDLManager {
private:
    SDL2pp::SDL sdl;
    SDL2pp::SDLTTF ttf;
    SDL2pp::SDLMixer mix;
    SDL2pp::Mixer mixer;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    ResourceManager resources;
    Camera camera;
    Sounds sounds;
    Animations animations;
    UpgradeShop shop;
    std::optional<SDL2pp::Texture> minimap;
    std::vector<SDL2pp::Rect> bridges_rects;
    std::vector<SDL2pp::Rect> decorations_rects;
    int iteration_counter = 0;
    std::string map_name;
    std::string decoration;
    std::string bridges;
    LeaderboardInfo leaderboard_info;
    SDL2pp::Texture* waiting_room_background;


    std::vector<SDL2pp::Rect> loadRects(const nlohmann::json& data);

    void update_camera(int player_x, int player_y);
    SDL2pp::Rect get_view_rect_with_shake();

    SDL2pp::Rect get_car_sprite_rect(const std::string& model) const;

    void render_player(const CarPositionDTO& player, const std::string& model, bool is_moving,
                       const SDL2pp::Rect& view_rect, double scale_x, double scale_y,
                       SDL2pp::Texture& cars_texture);
    void render_npc(const NpcDTO& npc, const SDL2pp::Rect& view_rect, double scale_x,
                    double scale_y, SDL2pp::Texture& cars_texture);
    void render_hud_health(int health, int max_health);
    void render_hud_minimap(int player_x, int player_y);
    void render_layer(const std::vector<SDL2pp::Rect>& rects, const SDL2pp::Rect& view_rect,
                      double scale_x, double scale_y, SDL2pp::Texture& map_texture);
    void render_bridges(const SDL2pp::Rect& view_rect, double scale_x, double scale_y,
                        SDL2pp::Texture& map_texture);
    void render_decorations(const SDL2pp::Rect& view_rect, double scale_x, double scale_y,
                            SDL2pp::Texture& map_texture);
    void render_checkpoints(const CheckpointDTO& checkpoint, const SDL2pp::Rect& view_rect,
                            double scale_x, double scale_y);
    double calculate_angle_to_checkpoint(const CarPositionDTO& player,
                                         const CheckpointDTO& checkpoint);
    void render_hint(const CarPositionDTO& player, const SDL2pp::Rect& view_rect, double scale_x,
                     double scale_y, SDL2pp::Texture& arrow_texture);

public:
    SDLManager();

    void load_bridges_rects(const nlohmann::json& bridges_rects_data);
    void load_decorations_rects(const nlohmann::json& decorations_rects_data);

    void set_minimap();
    void clear_screen();
    void render(LocalInfo& game_state);
    void render_waiting();
    void render_countdown(const LocalInfo& game_state);
    void render_leaderboard(const LeaderboardDTO& leaderboard, const std::string& title);
    void render_center_box(const std::string& text, const SDL2pp::Color& border_color);
    void show_screen();
    void draw_row(const std::string& sentence, int x, int y, const SDL2pp::Color& color,
                  int font_size);
    void render_stats(const PlayerInfo& player);
    nlohmann::json load_json_file(const std::string& path);
    std::optional<ButtonType> interact_button(int x, int y, int time);

    void load_city(const std::string& city_name);
    void render_shop(int time, int time_penalty, int upgrades_remaining,
                     int upgrade_shop_countdown);

    void close_shop();
    bool is_shop_open();
    void toggle_shop();

    void close_leaderboard();
    void open_leaderboard();
    bool is_leaderboard_open();

    void play_upgrade_sound();
    void play_finished_race_sound();
    void render_players(double scale_x, double scale_y, const SDL2pp::Rect& view_rect,
                        const CarPositionDTO& my_car, const PlayerInfo& my_player,
                        const LocalInfo& game_state, SDL2pp::Texture& cars_texture,
                        SDL2pp::Texture& map, SDL2pp::Texture& arrow_texture);

    void set_sound_info(const std::vector<uint32_t>& ids);

    void play_background_music(const std::string& path, int loops = -1);
    void stop_background_music();
    void set_music_volume(int volume);
    void render_text(const std::string& text, const SDL2pp::Color& text_color, int size);
};

#endif
