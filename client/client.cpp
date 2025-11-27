#include "client.h"

#include <any>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/car_position_dto.h"
#include "../common/game_update_dto.h"
#include "../common/player_dto.h"

#include "game.h"
#include "menu_interface.h"

Client::Client(const std::string& hostname, const std::string& servname):
        client_socket(hostname.c_str(), servname.c_str()),
        protocol(client_socket),
        menu(protocol) {}

void Client::run() {
    // lobby ui a
    PlayerDTO player("", -1, CarDTO(), -1, false);
    bool exit_game = false;
    do
    {
        menu.lobby(player);
        if (static_cast<int>(player.id) == -1) {
            exit_game = true;
            continue;
        }
    
        Game(protocol, player.id).run();
    } while (!exit_game);
}
