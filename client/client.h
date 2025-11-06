#ifndef CLIENT_H
#define CLIENT_H
#include <QApplication>
#include <map>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/car_position_dto.h"

#include "client_protocol.h"
#include "input_manager.h"
#include "menu_interface.h"
#include "receiver.h"
#include "sender.h"

class Client {
public:
    explicit Client(const std::string& hostname, const std::string& servname);
    void run();
    ~Client();

private:
    ClientProtocol protocol;
    Receiver receiver;
    InputManager input_manager;
    MenuInterface menu;
    uint32_t client_id;

    void draw_background(SDL2pp::Renderer& renderer, SDL2pp::Texture& background,
                         const SDL2pp::Rect& view_rect);

    void draw_car(SDL2pp::Renderer& renderer, SDL2pp::Texture& sprites, int pos_x, int pos_y,
                  const SDL2pp::Rect& view_rect, int angle);

    void draw_health_bar(SDL2pp::Renderer& renderer, int health);

    void draw_minimap(SDL2pp::Renderer& renderer, SDL2pp::Texture& minimap_static, int pos_x,
                      int pos_y, int bg_width, int bg_height, int minimap_width,
                      int minimap_height);
};

#endif  // CLIENT_H
