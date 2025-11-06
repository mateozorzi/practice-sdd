#ifndef SDL_MANAGER_H
#define SDL_MANAGER_H
#include <map>

#include "SDL2pp/Renderer.hh"
#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "common/car_position_dto.h"

#include "camera.h"


class SDLManager {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    Camera camera;
    std::optional<SDL2pp::Texture> map;
    std::optional<SDL2pp::Texture> sprite;
    std::optional<SDL2pp::Texture> minimap;


    void update_camera(int player_x, int player_y);
    void render_player(CarPositionDTO& player);
    void render_hud_health(int health);
    void render_hud_minimap(int player_x, int player_y);

public:
    SDLManager();
    void set_map();
    void set_sprite();
    void set_minimap();
    void clear_screen();
    void render(uint32_t client_id, int health, std::map<uint32_t, CarPositionDTO> cars);
    void show_screen();
};

#endif
