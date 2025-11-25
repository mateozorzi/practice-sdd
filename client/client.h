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

private:
    Socket client_socket;
    ClientProtocol protocol;
    MenuInterface menu;
};

#endif  // CLIENT_H
