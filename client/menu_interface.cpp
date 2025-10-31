#include "menu_interface.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <QApplication>
#include "Lobby/lobby.h"
#include "Lobby/selectcars.h"
#include <QString>
#include <QEventLoop>
#include "../common/car_dto.h"

MenuInterface::MenuInterface(ClientProtocol& protocol) : protocol(protocol) {}


bool MenuInterface::car_selection(){
    SelectCars sc;
    sc.showMaximized();

    bool car_selection = false;

    //server sends market of cars to choose
    //client send information to sc about cars to show in the ui
    std::vector<car_dto> market = std::vector<car_dto>{
        car_dto("Car A", 10, 5, 5, 3),
        car_dto("Car B", 15, 2, 6, 4),
        car_dto("Car C", 20, 10, 7, 5)
    };
    sc.set_market(market);

    QEventLoop loop;

    //confirm signal, the client select the car
    QObject::connect(&sc, &SelectCars::confirm_car, [&](const QString &car) {
        // client confirm the car selection
        std::string selected_car = car.toStdString();
        std::cout << "Car selected: " << selected_car << std::endl;
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

bool MenuInterface::lobby(){    
    int fake_argc = 0;
    char *fake_argv[] = { nullptr };
    QApplication app(fake_argc, fake_argv);
    
    Lobby l;
    l.showMaximized();

    std::string username;
    bool close_game = false;

    //QEventLoop loop;

    // Conectás la señal del lobby con el cliente
    QObject::connect(&l, &Lobby::input_username, [&](const QString &name){
        username = name.toStdString();
        std::cout << "Username entered: " << username << std::endl;

        protocol.send_username(username);

        //server send car market if username valid -> new ui where client choose car -> ui car upgrades -> return false
        l.hide();
        bool car_selected = car_selection();

        //car no selected, client close the game
        if (!car_selected) {
            close_game = true;
            app.quit();
        }else{
            //car selected, start the game
            close_game = false;
            app.quit();
        }
        
        //if not, response with an error id and ask again

    });

    // exit signal
    QObject::connect(&l, &Lobby::game_close, [&]() {
        // client close the game
        close_game = true;
        app.quit();
    });

    app.exec();

    return close_game;
}
