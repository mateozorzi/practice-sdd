#include "menu_interface.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <QApplication>
#include "Lobby/lobby.h"
#include "Lobby/selectcars.h"
#include "Lobby/creategame.h"
#include "Lobby/createlobby.h"
#include "Lobby/joinlobby.h"
#include <QString>
#include <QEventLoop>
#include "../common/car_dto.h"
#include "../common/car_catalogue_dto.h"
#include "../common/player_dto.h"
#include "../common/lobby_dto.h"

MenuInterface::MenuInterface(ClientProtocol& protocol) : protocol(protocol) {}


bool MenuInterface::car_selection(PlayerDTO& player){
    //server send market of cars
    protocol.ask_for_catalogue();
    CarCatalogueDTO catalogue = protocol.receive_car_catalogue();

    //CarCatalogueDTO catalogue(std::move(cars));
    std::cout << "Received car catalogue of size: " << static_cast<int>(catalogue.size) << std::endl;
    SelectCars sc(catalogue);
    sc.showMaximized();

    bool car_selection = false;

    QEventLoop loop;

    //confirm signal, the client select the car
    QObject::connect(&sc, &SelectCars::confirm_car, [&](const car_dto& car) {
        // client confirm the car selection
        player.car = car;
        std::cout << "Car selected: " << player.car.model << std::endl;
        //client send car_selected to server
        protocol.send_car_selection(car);
        car_selection = true;
        loop.quit();
    });


    //exit signal, the client close the car selection menu, go back to lobby
    QObject::connect(&sc, &SelectCars::back_to_lobby, [&]() {
        // client exit the car selection menu
        car_selection = false;
        loop.quit();
    });

    loop.exec();

    return car_selection;
}

bool MenuInterface::create_lobby(PlayerDTO& player) {
    //create lobby ui
    CreateLobby cl;
    cl.showMaximized();

    bool closed = false;

    QEventLoop loop;

    //create lobby signal
    QObject::connect(&cl, &CreateLobby::create_lobby, [&]() {
        //save info of ther lobby, and send to server
        std::string lobby_name = cl.get_lobby_name().toStdString();
        int max_players = cl.get_number_players();
        std::cout <<player.username<< " creating lobby: " << lobby_name << " with max players: " << max_players << std::endl;

        //protocol.send_create_lobby(LobbyDTO(player.id, player.username, lobby_name, max_players));

        loop.quit();

    });

    //close menu signal
    QObject::connect(&cl, &CreateLobby::close_menu, [&]() {
        closed = true;
        loop.quit();
    });

    loop.exec();

    return closed;
    
}

bool MenuInterface::join_lobby(PlayerDTO& player) {
    // join lobby ui
    JoinLobby jl;
    jl.showMaximized();

    bool closed = false;

    //server sends available lobbies
    QEventLoop loop;

    //join lobby signal


    //refresh lobbies signal
    QObject::connect(&jl, &JoinLobby::refresh, [&]() {
        //request lobbies to server and update ui

        std::cout <<player.username <<" refreshing available lobbies..." << std::endl;

        //server sends available lobbies
    });

    //close menu signal
    QObject::connect(&jl, &JoinLobby::close_menu, [&]() {
        closed = true;
        loop.quit();
    });
    
    loop.exec();

    return closed;

}

bool MenuInterface::lobby_create(PlayerDTO& player) {
    creategame cg;
    cg.showMaximized();

    bool closed = false;

    //client have two options: create or join lobby
    QEventLoop loop;

    //create lobby signal
    QObject::connect(&cg, &creategame::create_lobby, [&]() {
        cg.hide();
        
        bool close_menu = false;
        
        close_menu = create_lobby(player);

        if (close_menu){
            cg.show();
        }else{
            cg.close();

            loop.quit();
        }
    });

    //join lobby signal
    QObject::connect(&cg, &creategame::join_lobby, [&]() {
        cg.hide();
        bool close_menu = false;
        
        close_menu = join_lobby(player);

        if (close_menu){
            cg.show();
        }else{
            cg.close();

            loop.quit();

        }
    });

    //close menu signal
    QObject::connect(&cg, &creategame::close_menu, [&]() {
        closed = true;
        loop.quit();
    });

    loop.exec();

    return closed;
}

PlayerDTO MenuInterface::lobby(){    
    int fake_argc = 0;
    char *fake_argv[] = { nullptr };
    QApplication app(fake_argc, fake_argv);
    
    Lobby l;
    l.showMaximized();

    PlayerDTO player("", -1, car_dto());

    std::string username;
    uint32_t client_id = -1;

    bool menu_closed = false;

    //QEventLoop loop;

    // Conectás la señal del lobby con el cliente
    QObject::connect(&l, &Lobby::input_username, [&](const QString &name){
        username = name.toStdString();

        protocol.send_username(username);
        client_id = protocol.receive_id(); // wait for server response
        player.username = username;
        player.id = client_id;
        std::cout << "Username entered: " << player.username << std::endl;
        std::cout << "Received client ID: " << player.id << std::endl;

        
        //server send car market if username valid -> enter lobby (create or join)
        l.hide();
        menu_closed = lobby_create(player);

        if (menu_closed){
            //show enter username again
            l.show();
        }else{
            //new ui where client choose car -> ui car upgrades -> return false
            bool car_selected = car_selection(player);

            //car no selected, client close the game
            if (!car_selected) {
                //show enter username again
                l.show();
            }else{
                //car selected, start the game
                l.close();
                app.quit();
            }
        }

    });

    // exit signal
    QObject::connect(&l, &Lobby::game_close, [&]() {
        // client close the game
        player.id = -1;
        l.close();
        app.quit();
    });

    app.exec();

    return player;
}
