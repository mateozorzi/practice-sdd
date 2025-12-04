#ifndef UPGRADE_SHOP_H
#define UPGRADE_SHOP_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/client_event_dto.h"

#include "camera.h"
#include "resource_manager.h"
#include "sounds.h"

struct ShopButton {
    SDL2pp::Rect rect;
    std::string text;
    ButtonType type;
    int cost = 0;
};

struct ShopInfoDTO {
    int health;
    int speed;
    int acceleration;
    int handling;
};

enum class Car { DEFAULT };

class UpgradeShop {
private:
    SDL2pp::Renderer& renderer;
    SDL2pp::Mixer& mixer;
    ResourceManager& resources;
    Sounds& sounds;
    std::string font_path;

    std::vector<ShopButton> buttons;
    SDL2pp::Rect shop_rect;
    SDL2pp::Rect time_rect;
    SDL2pp::Rect car_rect;
    bool isOpen;
    int window_width;
    int window_height;
    ButtonType touched_button_type = None;
    ButtonType last_touched_button_type = None;

public:
    UpgradeShop(SDL2pp::Renderer& renderer, SDL2pp::Mixer& mixer, ResourceManager& resources,
                Sounds& sounds, int window_width, int window_height,
                const std::string& font_path =
                        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");

    void set_shop_info(const ShopInfoDTO& info);
    void render(int player_time, Car player_car, const std::string& map_name, const Camera& camera, int time_penalty, int upgrades_remaining);
    bool is_shop_open() const;
    void toggle_shop();
    std::optional<ButtonType> interact_button(int x, int y, int time);
    void close_shop();
};


#endif
