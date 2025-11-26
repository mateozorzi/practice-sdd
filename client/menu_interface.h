#ifndef MENU_INTERFACE_H
#define MENU_INTERFACE_H

#include "../common/player_dto.h"

#include "client_protocol.h"

class MenuInterface {
private:
    ClientProtocol& protocol;

public:
    explicit MenuInterface(ClientProtocol& protocol);
    PlayerDTO lobby();
    bool car_selection(PlayerDTO& player);
    bool waiting_room(const bool is_owner, const LobbyDTO& lobby);
    bool lobby_create(PlayerDTO& player);
    bool select_races();
    bool create_lobby(const PlayerDTO& player);
    bool join_lobby();
};


#endif  // MENU_INTERFACE_H
