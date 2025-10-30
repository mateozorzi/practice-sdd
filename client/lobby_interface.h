#ifndef LOBBY_INTERFACE_H
#define LOBBY_INTERFACE_H

#include <QApplication>

class LobbyInterface {
private:
    QApplication app;

public:
    LobbyInterface();
    ~LobbyInterface();

    void show();
    void close();

};

#endif  // LOBBY_INTERFACE_H
