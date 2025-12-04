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
        menu(protocol, 0, {nullptr}) {}

void Client::run() {

    bool server_connected = true;

    while (server_connected) {

        PlayerDTO player;
        menu.lobby(player);
        if (static_cast<int>(player.id) == -1) {
            break;
        }

        Game game(protocol, player.id);
        server_connected = game.run();
    }
}
