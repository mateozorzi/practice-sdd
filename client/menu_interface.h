#ifndef MENU_INTERFACE_H
#define MENU_INTERFACE_H

#include "client_protocol.h"

#include "../common/player_dto.h"

class MenuInterface {
private:
    ClientProtocol& protocol;

public:
    explicit MenuInterface(ClientProtocol& protocol);
    PlayerDTO lobby();
    bool car_selection(PlayerDTO& player);
};


#endif  // MENU_INTERFACE_H
