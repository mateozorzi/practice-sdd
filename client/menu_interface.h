#ifndef MENU_INTERFACE_H
#define MENU_INTERFACE_H

#include "client_protocol.h"

class MenuInterface {
private:
    ClientProtocol& protocol;

public:
    MenuInterface(ClientProtocol& protocol);
    bool lobby();
    bool car_selection();

};


#endif // MENU_INTERFACE_H
