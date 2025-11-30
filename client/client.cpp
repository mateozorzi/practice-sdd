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
    bool open_lobby = true;
    while(open_lobby){
        PlayerDTO player;
        menu.lobby(player);
        if (static_cast<int>(player.id) == -1) {
            open_lobby = false;
            continue;
        }

        Game game(protocol, player.id);
        game.run();
        std::cout<<"Client game ended, returning to lobby menu."<<std::endl;
    }
}
