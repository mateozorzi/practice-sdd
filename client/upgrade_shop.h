#ifndef UPGRADE_SHOP_H
#define UPGRADE_SHOP_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

enum ButtonType { Health, Speed, Acceleration, Driveability };

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
    int driveability;
};

enum class Car { DEFAULT };

class UpgradeShop {
private:
    SDL2pp::Renderer& renderer;
    SDL2pp::SDLTTF ttf;
    SDL2pp::Font font;

    std::vector<ShopButton> buttons;
    SDL2pp::Rect shop_rect;
    SDL2pp::Rect time_rect;
    SDL2pp::Rect car_rect;
    bool isOpen;
    int window_width;
    int window_height;

public:
    UpgradeShop(SDL2pp::Renderer& renderer, int window_width, int window_height,
                const std::string& font_path =
                        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");

    void set_shop_info(const ShopInfoDTO& info);

    void render(int player_time, Car player_car);

    bool is_shop_open() const;

    void toggle_shop();
};


#endif
