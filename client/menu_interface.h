#ifndef MENU_INTERFACE_H
#define MENU_INTERFACE_H

#include <QApplication>
#include <QAudioOutput>
#include <QMediaPlayer>

#include "../common/player_dto.h"

#include "client_protocol.h"

class MenuInterface {
private:
    ClientProtocol& protocol;
    QApplication app;
    QMediaPlayer* musicPlayer;
    QAudioOutput* audioOutput;

public:
    MenuInterface(ClientProtocol& protocol, int argc, char** argv);
    void set_music(QObject* parent);
    void lobby(PlayerDTO& player);
    bool car_selection(PlayerDTO& player);
    bool lobby_create(const PlayerDTO& player);
    bool select_races();
    bool create_lobby(const PlayerDTO& player);
    bool join_lobby();
};


#endif  // MENU_INTERFACE_H
