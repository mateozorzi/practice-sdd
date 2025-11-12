#ifndef SDL_MANAGER_H
#define SDL_MANAGER_H
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "SDL2pp/Renderer.hh"
#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "common/car_position_dto.h"
#include "common/player_dto.h"

#include "camera.h"

static const std::unordered_map<std::string, SDL2pp::Rect> CAR_SPRITES = {
        {"Compact", SDL2pp::Rect(128, 32, 32, 32)},      {"Sport", SDL2pp::Rect(166, 104, 26, 40)},
        {"Convertible", SDL2pp::Rect(166, 184, 26, 38)}, {"Sedan", SDL2pp::Rect(166, 264, 26, 38)},
        {"Jeep", SDL2pp::Rect(166, 345, 32, 38)},        {"Rally", SDL2pp::Rect(166, 424, 32, 38)},
        {"Truck", SDL2pp::Rect(203, 512, 24, 49)}};

class SDLManager {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    Camera camera;
    std::optional<SDL2pp::Texture> map;
    std::optional<SDL2pp::Texture> sprite;
    std::optional<SDL2pp::Texture> minimap;
    std::vector<SDL2pp::Rect> bridges_rects;

    std::vector<SDL2pp::Rect> loadRects(const std::string& path);

    void update_camera(int player_x, int player_y);
    void render_player(CarPositionDTO& player, const std::string& model);
    void render_hud_health(int health);
    void render_hud_minimap(int player_x, int player_y);
    void render_bridges();

public:
    SDLManager();
    void set_map();
    void set_sprite();
    void set_minimap();
    void clear_screen();
    void render(const PlayerDTO& player, int health, std::map<uint32_t, CarPositionDTO> cars);
    void show_screen();
};

#endif
