#ifndef MENU_CONNECTION_H
#define MENU_CONNECTION_H

#include <string>
#include <utility>

#include "Lobby/connectioninfo.h"

class MenuConnection {
public:
    MenuConnection();
    std::pair<std::string, std::string> run();
};

#endif
