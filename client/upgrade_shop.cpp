#include "upgrade_shop.h"

#include <optional>
#include <string>

#include "camera.h"
#include "client_constants.h"

UpgradeShop::UpgradeShop(SDL2pp::Renderer& renderer, SDL2pp::Mixer& mixer,
                         ResourceManager& resources, Sounds& sounds, int window_width,
                         int window_height, const std::string& font_path):
        renderer(renderer),
        mixer(mixer),
        resources(resources),
        sounds(sounds),
        font_path(font_path),
        shop_rect((window_width - window_width * SHOP_SIZE_RATIO_NUM / SHOP_SIZE_RATIO_DEN) / 2,
                  (window_height - window_height * SHOP_SIZE_RATIO_NUM / SHOP_SIZE_RATIO_DEN) / 2,
                  window_width * SHOP_SIZE_RATIO_NUM / SHOP_SIZE_RATIO_DEN,
                  window_height * SHOP_SIZE_RATIO_NUM / SHOP_SIZE_RATIO_DEN),
        isOpen(false),
        window_width(window_width),
        window_height(window_height) {

    // Tamaño: 3/4 de la ventana
    int shop_width = window_width * SHOP_SIZE_RATIO_NUM / SHOP_SIZE_RATIO_DEN;
    int shop_height = window_height * SHOP_SIZE_RATIO_NUM / SHOP_SIZE_RATIO_DEN;

    // Botones
    int button_width = shop_width * SHOP_BUTTON_WIDTH_RATIO_NUM / SHOP_BUTTON_WIDTH_RATIO_DEN;
    int button_height = shop_height / SHOP_BUTTON_HEIGHT_DIVISOR;
    int button_spacing = shop_height / SHOP_BUTTON_SPACING_DIVISOR;

    std::string buttons_text[4] = {"Health", "Speed", "Acceleration", "Handling"};
    ButtonType buttons_type[4] = {Health, Speed, Acceleration, Handling};

    int start_y = shop_rect.y + shop_height / SHOP_BUTTON_START_Y_DIVISOR;
    int button_x = shop_rect.x + (shop_width - button_width) / 2;  // Centrar horizontalmente

    for (int i = 0; i < 4; i++) {
        int button_y = start_y + i * (button_height + button_spacing);
        SDL2pp::Rect button_rect(button_x, button_y, button_width, button_height);
        buttons.push_back({button_rect, buttons_text[i], buttons_type[i]});
    }
}

void UpgradeShop::set_shop_info(const ShopInfoDTO& info) {
    for (auto& button: buttons) {
        switch (button.type) {
            case Health:
                button.cost = info.health;
                break;
            case Speed:
                button.cost = info.speed;
                break;
            case Acceleration:
                button.cost = info.acceleration;
                break;
            case Handling:
                button.cost = info.handling;
                break;
            default:
                break;
        }
    }
}

void UpgradeShop::render(int player_time, Car player_car, const std::string& map_name,
                         const Camera& camera, int time_penalty, int upgrades_remaining,
                         int upgrade_shop_countdown) {
    (void)player_time;  // Aumenta segun compre mejoras
    (void)player_car;   // Para poner el auto al lado de botones de mejoras

    // No renderizar si esta cerrado
    if (!isOpen) {
        return;
    }

    SDL2pp::Texture& map = resources.get_texture(map_name);
    SDL2pp::Rect view_rect = camera.get_view_rect();
    // in the bacground the city map is rendered, not render all the map, only a portion
    renderer.Copy(map, view_rect, SDL2pp::Rect(0, 0, window_width, window_height));

    SDL2pp::Color shop_color(COLOR_SHOP_BG_R, COLOR_SHOP_BG_G, COLOR_SHOP_BG_B, COLOR_SHOP_BG_A);
    SDL2pp::Color border_color(COLOR_BLACK_R, COLOR_BLACK_G, COLOR_BLACK_B, COLOR_BLACK_A);
    SDL2pp::Color button_color(COLOR_SHOP_BUTTON_R, COLOR_SHOP_BUTTON_G, COLOR_SHOP_BUTTON_B,
                               COLOR_SHOP_BUTTON_A);
    SDL2pp::Color button_hover_color(COLOR_SHOP_BUTTON_HOVER_R, COLOR_SHOP_BUTTON_HOVER_G,
                                     COLOR_SHOP_BUTTON_HOVER_B, COLOR_SHOP_BUTTON_HOVER_A);
    int border_thickness = SHOP_BORDER_THICKNESS;

    // Fondo de tienda
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(shop_color);
    renderer.FillRect(shop_rect);

    // Bordes de tienda
    renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
    renderer.SetDrawColor(border_color);
    renderer.FillRect(SDL2pp::Rect(shop_rect.x, shop_rect.y, shop_rect.w, border_thickness));
    renderer.FillRect(SDL2pp::Rect(shop_rect.x, shop_rect.y + shop_rect.h - border_thickness,
                                   shop_rect.w, border_thickness));
    renderer.FillRect(SDL2pp::Rect(shop_rect.x, shop_rect.y, border_thickness, shop_rect.h));
    renderer.FillRect(SDL2pp::Rect(shop_rect.x + shop_rect.w - border_thickness, shop_rect.y,
                                   border_thickness, shop_rect.h));

    // Titulo de la tienda
    SDL2pp::Color title_color(COLOR_GOLD_R, COLOR_GOLD_G, COLOR_GOLD_B, COLOR_GOLD_A);
    SDL2pp::Texture& title_texture =
            resources.get_text(SHOP_TITLE_TEXT, font_path, SHOP_FONT_SIZE, title_color);
    int title_x = shop_rect.x + (shop_rect.w - title_texture.GetWidth()) / 2;
    int title_y = shop_rect.y + SHOP_TITLE_Y_OFFSET;
    renderer.Copy(
            title_texture, SDL2pp::NullOpt,
            SDL2pp::Rect(title_x, title_y, title_texture.GetWidth(), title_texture.GetHeight()));

    //info penalty
    std::string penalty_text = "Time penalty per Upgrade :  " + std::to_string(time_penalty) + " seg";
    SDL2pp::Color penalty_color(139, 0, 0, 255);  // DarkRed estándar


    SDL2pp::Texture& penalty_texture =
            resources.get_text(penalty_text, font_path, UPGRADE_COUNTDOWN_FONT_SIZE, penalty_color);

    int penalty_x = shop_rect.x + (shop_rect.w - penalty_texture.GetWidth()) / 2;
    int penalty_y = title_y + title_texture.GetHeight() + 20;  // separacion del título

    renderer.Copy(penalty_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(penalty_x, penalty_y, penalty_texture.GetWidth(),
                               penalty_texture.GetHeight()));

    //info max upgrades
    std::string max_upgrades_text = "Upgrades remaining :  " + std::to_string(upgrades_remaining);
    SDL2pp::Color max_upgrades_color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);

    SDL2pp::Texture& max_upgrades_texture =
            resources.get_text(max_upgrades_text, font_path, INFO_FONT_SIZE, max_upgrades_color);

    int max_upgrades_x = shop_rect.x + (shop_rect.w - max_upgrades_texture.GetWidth()) / 2;
    int max_upgrades_y = penalty_y + penalty_texture.GetHeight() + 10;  // separacion de la info

    renderer.Copy(max_upgrades_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(max_upgrades_x, max_upgrades_y, max_upgrades_texture.GetWidth(),
                               max_upgrades_texture.GetHeight()));

    // Botones
    for (const ShopButton& btn: buttons) {
        // Fondo del boton
        renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
        // Hover si el mouse esta por encima
        if (touched_button_type == btn.type) {
            renderer.SetDrawColor(button_hover_color);
        } else {
            renderer.SetDrawColor(button_color);
        }
        renderer.FillRect(btn.rect);

        // Borde del boton
        renderer.SetDrawColor(border_color);
        int btn_border = SHOP_BUTTON_BORDER_THICKNESS;
        renderer.FillRect(SDL2pp::Rect(btn.rect.x, btn.rect.y, btn.rect.w, btn_border));
        renderer.FillRect(SDL2pp::Rect(btn.rect.x, btn.rect.y + btn.rect.h - btn_border, btn.rect.w,
                                       btn_border));
        renderer.FillRect(SDL2pp::Rect(btn.rect.x, btn.rect.y, btn_border, btn.rect.h));
        renderer.FillRect(SDL2pp::Rect(btn.rect.x + btn.rect.w - btn_border, btn.rect.y, btn_border,
                                       btn.rect.h));

        // Texto del boton
        if (!btn.text.empty()) {
            std::string button_text =
                    btn.text /*+ ": " + std::to_string(btn.cost) + SHOP_SECONDS_TEXT*/;
            SDL2pp::Color text_color(COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, COLOR_WHITE_A);
            SDL2pp::Texture& text_texture = resources.get_text(
                    button_text, font_path, LEADERBOARD_HEADER_FONT_SIZE, text_color);

            int text_x = btn.rect.x + (btn.rect.w - text_texture.GetWidth()) / 2;
            int text_y = btn.rect.y + (btn.rect.h - text_texture.GetHeight()) / 2;

            renderer.Copy(text_texture, SDL2pp::NullOpt,
                          SDL2pp::Rect(text_x, text_y, text_texture.GetWidth(),
                                       text_texture.GetHeight()));
        }
    }

    std::string countdown_text =
        "The countdown starts in :  " + std::to_string(upgrade_shop_countdown+1) + " seg";

    SDL2pp::Texture& countdown_texture =
            resources.get_text(countdown_text, font_path, UPGRADE_COUNTDOWN_FONT_SIZE, title_color);

    int max_btn_bottom = 0;
    for (const ShopButton& btn: buttons) {
        int bottom = btn.rect.y + btn.rect.h;
        if (bottom > max_btn_bottom)
            max_btn_bottom = bottom;
    }

    int countdown_x = shop_rect.x + (shop_rect.w - countdown_texture.GetWidth()) / 2;
    int countdown_y = max_btn_bottom + 30;  // margen debajo de los botones

    renderer.Copy(countdown_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(countdown_x, countdown_y, countdown_texture.GetWidth(),
                               countdown_texture.GetHeight()));
}

bool UpgradeShop::is_shop_open() const { return isOpen; }

void UpgradeShop::toggle_shop() { isOpen = !isOpen; }

void UpgradeShop::close_shop() { isOpen = false; }

std::optional<ButtonType> UpgradeShop::interact_button(int x, int y, int time) {
    SDL2pp::Point point{x, y};
    touched_button_type = None;

    for (const ShopButton& btn: buttons) {
        if (btn.rect.Contains(point)) {
            touched_button_type = btn.type;

            if (time < btn.cost) {
                return std::nullopt;
            }

            return btn.type;
        }
    }

    return std::nullopt;
}
