#include "upgrade_shop.h"

#include <optional>

UpgradeShop::UpgradeShop(SDL2pp::Renderer& renderer, SDL2pp::Mixer& mixer, ResourceManager& resources, Sounds& sounds, int window_width, int window_height,
                         const std::string& font_path) :
        renderer(renderer),
        mixer(mixer),
        resources(resources),
        sounds(sounds),
        font(font_path, 18),
        shop_rect((window_width - window_width * 3 / 4) / 2,
                  (window_height - window_height * 3 / 4) / 2, window_width * 3 / 4,
                  window_height * 3 / 4),
        isOpen(true),
        window_width(window_width),
        window_height(window_height) {

    // Tamaño: 3/4 de la ventana
    int shop_width = window_width * 3 / 4;
    int shop_height = window_height * 3 / 4;

    // Botones
    int button_width = shop_width * 3 / 5;
    int button_height = shop_height / 8;
    int button_spacing = shop_height / 20;

    std::string buttons_text[4] = {"Health", "Speed", "Acceleration", "Driveability"};
    ButtonType buttons_type[4] = {Health, Speed, Acceleration, Driveability};

    int start_y = shop_rect.y + shop_height / 4;  // Empieza 1/4 de altura de la ventana de tienda
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
            case Driveability:
                button.cost = info.driveability;
                break;
            default:
                break;
        }
    }
}

void UpgradeShop::render(int player_time, Car player_car) {
    (void)player_time;  // Aumenta segun compre mejoras
    (void)player_car;   // Para poner el auto al lado de botones de mejoras

    // No renderizar si esta cerrado
    if (!isOpen) {
        return;
    }

    SDL2pp::Color shop_color(40, 40, 40, 230);
    SDL2pp::Color border_color(0, 0, 0);
    SDL2pp::Color button_color(80, 80, 80, 255);
    int border_thickness = 5;

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
    SDL2pp::Surface title_surface =
            font.RenderText_Blended("UPGRADE SHOP", SDL_Color{255, 215, 0, 255});
    SDL2pp::Texture title_texture(renderer, title_surface);
    int title_x = shop_rect.x + (shop_rect.w - title_texture.GetWidth()) / 2;
    int title_y = shop_rect.y + 15;
    renderer.Copy(
            title_texture, SDL2pp::NullOpt,
            SDL2pp::Rect(title_x, title_y, title_texture.GetWidth(), title_texture.GetHeight()));

    // Botones
    for (const ShopButton& btn: buttons) {
        // Fondo del boton
        renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
        renderer.SetDrawColor(button_color);
        renderer.FillRect(btn.rect);

        // Borde del boton
        renderer.SetDrawColor(border_color);
        int btn_border = 2;
        renderer.FillRect(SDL2pp::Rect(btn.rect.x, btn.rect.y, btn.rect.w, btn_border));
        renderer.FillRect(SDL2pp::Rect(btn.rect.x, btn.rect.y + btn.rect.h - btn_border, btn.rect.w,
                                       btn_border));
        renderer.FillRect(SDL2pp::Rect(btn.rect.x, btn.rect.y, btn_border, btn.rect.h));
        renderer.FillRect(SDL2pp::Rect(btn.rect.x + btn.rect.w - btn_border, btn.rect.y, btn_border,
                                       btn.rect.h));

        // Texto del boton
        if (!btn.text.empty()) {
            std::string button_text = btn.text + ": " + std::to_string(btn.cost) + " seconds";
            SDL2pp::Surface text_surface =
                    font.RenderText_Blended(button_text, SDL_Color{255, 255, 255, 255});
            SDL2pp::Texture text_texture(renderer, text_surface);

            int text_x = btn.rect.x + (btn.rect.w - text_texture.GetWidth()) / 2;
            int text_y = btn.rect.y + (btn.rect.h - text_texture.GetHeight()) / 2;

            renderer.Copy(text_texture, SDL2pp::NullOpt,
                          SDL2pp::Rect(text_x, text_y, text_texture.GetWidth(),
                                       text_texture.GetHeight()));
        }
    }
}

bool UpgradeShop::is_shop_open() const { return isOpen; }

void UpgradeShop::toggle_shop() { isOpen = !isOpen; }

void UpgradeShop::close_shop() {
    if (isOpen) {
        isOpen = false;
    }
}

std::optional<ButtonType> UpgradeShop::interact_button(int x, int y, int time) {
    SDL2pp::Point point{x, y};
    touched_button_type = ButtonType::None;

    for (const ShopButton& btn: buttons) {
        if (btn.rect.Contains(point)) {
            touched_button_type = btn.type;

            if (time < btn.cost) {
                return std::nullopt;
            }

            last_touched_button_type = btn.type;

            return btn.type;
        }
    }
    if (touched_button_type == ButtonType::None) {
        last_touched_button_type = ButtonType::None;
    }

    return std::nullopt;
}

