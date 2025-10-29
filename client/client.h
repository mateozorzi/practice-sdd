#ifndef CLIENT_H
#define CLIENT_H
#include "client_protocol.h"
#include "receiver.h"
#include "sender.h"
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>


class Client {
public:
    explicit Client(const std::string& hostname, const std::string& servname);
    void run();
    ~Client();

private:
    ClientProtocol protocol;
    Sender sender;
    Receiver receiver;

    void draw_background(SDL2pp::Renderer& renderer, SDL2pp::Texture& background,
        int camera_x, int camera_y, int view_width, int view_height);
    
    void draw_car(SDL2pp::Renderer& renderer, SDL2pp::Texture& sprites, int pos_x, int pos_y, int camera_x, int camera_y,
        int car_x, int car_y, int view_width, int view_height);
    
    void draw_health_bar(SDL2pp::Renderer& renderer, int health);
    
    void draw_minimap(SDL2pp::Renderer& renderer, SDL2pp::Texture& minimap_static, int pos_x, int pos_y, int bg_width, int bg_height,
        int minimap_width, int minimap_height);
};

#endif //CLIENT_H
