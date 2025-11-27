#ifndef MENU_INTERFACE_H
#define MENU_INTERFACE_H

#include "../common/player_dto.h"

#include "client_protocol.h"

class MenuInterface {
private:
    ClientProtocol& protocol;

public:
    explicit MenuInterface(ClientProtocol& protocol);
    void lobby(PlayerDTO& player);
    bool car_selection(PlayerDTO& player);
    bool lobby_create(const PlayerDTO& player);
    bool select_races();
    bool create_lobby(const PlayerDTO& player);
    bool join_lobby();
};


#endif  // MENU_INTERFACE_H
