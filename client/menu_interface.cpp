#include "menu_interface.h"

#include <QApplication>
#include <QAudioOutput>
#include <QEventLoop>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QUrl>
#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>

#include "../common/car_catalogue_dto.h"
#include "../common/car_dto.h"
#include "../common/lobby_catalogue_dto.h"
#include "../common/lobby_dto.h"
#include "../common/player_dto.h"
#include "Lobby/creategame.h"
#include "Lobby/createlobby.h"
#include "Lobby/joinlobby.h"
#include "Lobby/lobby.h"
#include "Lobby/raceselector.h"
#include "Lobby/selectcars.h"
#include "Lobby/waitingroom.h"

#include "client_constants.h"

MenuInterface::MenuInterface(ClientProtocol& protocol, int argc = 0, char** argv = nullptr):
        protocol(protocol), argc(argc), argv(argv), app(this->argc, this->argv) {}


bool MenuInterface::car_selection(PlayerDTO& player) {

    CarCatalogueDTO catalogue = protocol.receive_car_catalogue();

    SelectCars sc(catalogue);
    sc.showMaximized();

    bool car_selection = false;

    QEventLoop loop;

    QObject::connect(&sc, &SelectCars::confirm_car, [&](const CarDTO& car) {
        player.car = car;
        protocol.send_car_selection(car);
        car_selection = true;
        loop.quit();
    });

    QObject::connect(&sc, &SelectCars::back_to_lobby, [&]() {
        car_selection = false;
        protocol.send_client_action(ClientAction::DISCONNECT);
        protocol.receive_update();
        loop.quit();
    });

    loop.exec();

    return car_selection;
}

bool MenuInterface::select_races() {

    protocol.ask_for_race_catalogue();
    RaceCatalogueDTO races = protocol.receive_race_catalogue();

    RaceSelector sr(races);
    sr.showMaximized();

    bool closed = false;

    QEventLoop loop;

    QObject::connect(&sr, &RaceSelector::confirm_races, [&](const std::vector<RaceDTO>& races) {
        protocol.send_races_selection(races);
        sr.close();
        loop.quit();
    });

    QObject::connect(&sr, &RaceSelector::close_menu, [&]() {
        closed = true;
        loop.quit();
    });

    loop.exec();

    return closed;
}

bool MenuInterface::create_lobby(const PlayerDTO& player) {

    CreateLobby cl;
    cl.showMaximized();

    bool closed = false;

    QEventLoop loop;

    QObject::connect(&cl, &CreateLobby::create_lobby, [&]() {
        std::string lobby_name = cl.get_lobby_name().toStdString();
        int max_players = cl.get_number_players();
        LobbyDTO lobby(LobbyAction::CREATE_LOBBY, player.id, lobby_name, max_players);
        cl.hide();

        closed = select_races();

        if (closed) {

            loop.quit();
            return;
        }

        protocol.send_create_lobby(lobby);
        // client wait for server response
        LobbyDTO lobby_response = protocol.receive_lobby();
        if (lobby_response.action == LobbyAction::CREATE_OR_JOIN_SUCCESS) {
            cl.close();
            loop.quit();
        } else if (lobby_response.action == LobbyAction::CREATE_OR_JOIN_ERROR) {
            cl.show();
            cl.create_denied();
        } else if (lobby_response.action == LobbyAction::MAX_PLAYERS_ERROR) {
            // lobby full
            cl.show();
            cl.create_denied_players(lobby_response.max_players, "Max players reached");
        } else if (lobby_response.action == LobbyAction::MIN_PLAYERS_ERROR) {
            // not enough players
            cl.show();
            cl.create_denied_players(lobby_response.max_players, "Min players required");
        }
    });

    QObject::connect(&cl, &CreateLobby::close_menu, [&]() {
        closed = true;
        loop.quit();
    });

    loop.exec();

    return closed;
}

bool MenuInterface::join_lobby() {

    LobbyDTO lobby_request(LobbyAction::SEND_LOBBY_LIST);
    protocol.ask_for_lobbies(lobby_request);
    LobbyCatalogueDTO lobbies = protocol.receive_lobby_catalogue();


    JoinLobby jl(lobbies);
    jl.showMaximized();

    bool closed = false;

    QEventLoop loop;

    QObject::connect(&jl, &JoinLobby::join_lobby, [&](const LobbyDTO& lobby) {
        LobbyDTO lobby_req(LobbyAction::JOIN_LOBBY, lobby.lobby_id);
        protocol.send_action_lobby(lobby_req);
        LobbyDTO lobby_info = protocol.receive_lobby();

        if (lobby_info.action == LobbyAction::CREATE_OR_JOIN_SUCCESS) {
            // lobby accepted
            jl.close();
            loop.quit();

        } else if (lobby_info.action == LobbyAction::CREATE_OR_JOIN_ERROR) {
            // lobby denied
            jl.fill_table(lobbies);
            jl.join_denied();
        }
    });

    QObject::connect(&jl, &JoinLobby::refresh, [&]() {
        LobbyDTO lobby_request(LobbyAction::SEND_LOBBY_LIST);
        protocol.ask_for_lobbies(lobby_request);
        LobbyCatalogueDTO lobbies = protocol.receive_lobby_catalogue();

        jl.fill_table(lobbies);
    });

    // close menu signal
    QObject::connect(&jl, &JoinLobby::close_menu, [&]() {
        closed = true;
        loop.quit();
    });

    loop.exec();

    return closed;
}

bool MenuInterface::lobby_create(const PlayerDTO& player) {
    creategame cg;
    cg.showMaximized();

    bool closed = false;
    bool owner = false;

    // client have two options: create or join lobby
    QEventLoop loop;

    // create lobby signal
    QObject::connect(&cg, &creategame::create_lobby, [&]() {
        cg.hide();

        bool close_menu = false;

        close_menu = create_lobby(player);

        if (close_menu) {
            cg.show();
        } else {
            cg.close();

            owner = true;

            loop.quit();
        }
    });

    // join lobby signal
    QObject::connect(&cg, &creategame::join_lobby, [&]() {
        cg.hide();
        bool close_menu = false;

        close_menu = join_lobby();

        if (close_menu) {
            cg.show();
        } else {
            cg.close();

            loop.quit();
        }
    });

    // close menu signal
    QObject::connect(&cg, &creategame::close_menu, [&]() {
        closed = true;
        loop.quit();
    });

    loop.exec();

    return closed;
}

void MenuInterface::set_music(QObject* parent) {
    musicPlayer = new QMediaPlayer(parent);
    audioOutput = new QAudioOutput(parent);
    musicPlayer->setAudioOutput(audioOutput);

    musicPlayer->setSource(
            QUrl::fromLocalFile("/var/nfs/assets/music/Need for Speed II SE Soundtrack - Main Menu "
                                "and Showcase Theme I - RonTaboga.mp3"));

    audioOutput->setVolume(MUSIC_VOLUME);  // Volumen entre 0 y 100

    musicPlayer->setLoops(QMediaPlayer::Infinite);
    musicPlayer->play();
}

void MenuInterface::lobby(PlayerDTO& player) {
    QEventLoop loop;

    Lobby l;
    set_music(&l);
    l.showMaximized();

    std::string username;
    uint32_t client_id = -1;
    player.id = client_id;

    bool menu_closed = false;

    // Conectás la señal del lobby con el cliente
    QObject::connect(&l, &Lobby::input_username, [&](const QString& name) {
        username = name.toStdString();

        protocol.change_username(LobbyDTO(LobbyAction::CHANGE_USERNAME, client_id, username));
        client_id = protocol.receive_id();  // wait for server response
        player.username = username;
        player.id = client_id;


        // server send car market if username valid -> enter lobby (create or join)
        l.hide();
        menu_closed = lobby_create(player);
        if (menu_closed) {
            // show enter username again
            l.show();
        } else {
            // new ui where client choose car -> ui car upgrades -> return false
            bool car_selected = car_selection(player);

            // car no selected, client close the game
            if (!car_selected) {
                // show enter username again
                l.show();
            } else {
                // car selected, start the game
                l.close();
                loop.quit();
            }
        }
    });

    // exit signal
    QObject::connect(&l, &Lobby::game_close, [&]() {
        // client close the game
        protocol.send_exit(LobbyDTO(LobbyAction::EXIT_GAME));
        player.id = -1;

        l.close();
        loop.quit();
    });

    loop.exec();

    musicPlayer->stop();
}
