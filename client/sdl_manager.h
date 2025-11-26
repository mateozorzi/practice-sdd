#ifndef SDL_MANAGER_H
#define SDL_MANAGER_H
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "SDL2pp/Renderer.hh"
#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "common/car_position_dto.h"
#include "common/player_dto.h"

#include "animations.h"
#include "camera.h"
#include "game_state.h"
#include "resource_manager.h"
#include "sounds.h"
#include "upgrade_shop.h"

static const std::unordered_map<std::string, SDL2pp::Rect> CAR_SPRITES = {
        {"Compact", SDL2pp::Rect(0, 0, 50, 50)},       {"Sport", SDL2pp::Rect(50, 0, 50, 50)},
        {"Convertible", SDL2pp::Rect(100, 0, 50, 50)}, {"Sedan", SDL2pp::Rect(150, 0, 50, 50)},
        {"Jeep", SDL2pp::Rect(200, 0, 50, 50)},        {"Rally", SDL2pp::Rect(250, 0, 50, 50)},
        {"Truck", SDL2pp::Rect(300, 0, 50, 50)}};

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


    std::vector<SDL2pp::Rect> loadRects(const nlohmann::json& data);

    void update_camera(int player_x, int player_y);
    void render_player(const CarPositionDTO& player, const std::string& model, bool is_moving);
    void render_hud_health(int health, int max_health);
    void render_hud_minimap(int player_x, int player_y);
    void render_layer(const std::vector<SDL2pp::Rect>& rects);
    void render_bridges();
    void render_decorations();
    void render_checkpoints(const CheckpointDTO& checkpoint);
    double calculate_angle_to_checkpoint(const CarPositionDTO& player,
                                         const CheckpointDTO& checkpoint);
    void render_hint(const CarPositionDTO& player);

public:
    SDLManager();

    void load_bridges_rects(const nlohmann::json& bridges_rects_data);
    void load_decorations_rects(const nlohmann::json& decorations_rects_data);

    void set_minimap();
    void clear_screen();
    void render(LocalInfo& game_state);
    void show_screen();
    void draw_row(const std::string& sentence, int x, int y, SDL2pp::Color color, int font_size);
    void render_stats(const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& cars);
    nlohmann::json load_json_file(const std::string& path);
    std::optional<ButtonType> interact_button(int x, int y, int time);

    void load_city(const std::string& city_name);
    void render_shop(int time);

    void close_shop();
};

#endif
