#ifndef CLIENT_H
#define CLIENT_H


#include <QApplication>
#include <map>
#include <memory>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/car_position_dto.h"

#include "client_protocol.h"
#include "input_manager.h"
#include "menu_interface.h"
#include "receiver.h"
#include "sdl_manager.h"
#include "sender.h"


class Client {

public:
    explicit Client(const std::string& hostname, const std::string& servname);
    void run();
    ~Client();

private:
    Socket client_socket;
    std::unique_ptr<SDLManager> sdl_manager;
    ClientProtocol protocol;
    Receiver receiver;
    InputManager input_manager;
    MenuInterface menu;
    uint32_t client_id;
};

#endif  // CLIENT_H
