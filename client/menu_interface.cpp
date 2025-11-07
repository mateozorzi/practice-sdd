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
#include "../common/car_catalogue_dto.h"

MenuInterface::MenuInterface(ClientProtocol& protocol) : protocol(protocol) {}


bool MenuInterface::car_selection(){
    //server send market of cars
    protocol.ask_for_catalogue();
    CarCatalogueDTO catalogue = protocol.receive_car_catalogue();
    //std::vector<car_dto> cars = std::vector<car_dto>{
    //    car_dto("Compact", 10, 5, 5, 3),
    //    car_dto("Sport", 15, 2, 6, 4),
    //    car_dto("Convertible", 20, 10, 7, 5)
    //};

    //CarCatalogueDTO catalogue(std::move(cars));
    std::cout << "Received car catalogue of size: " << static_cast<int>(catalogue.size) << std::endl;
    SelectCars sc(catalogue);
    sc.showMaximized();

    bool car_selection = false;

    QEventLoop loop;

    //confirm signal, the client select the car
    QObject::connect(&sc, &SelectCars::confirm_car, [&](const car_dto& car) {
        // client confirm the car selection
        std::cout << "Car selected: " << car.model << std::endl;
        //client send car_selected to server
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
    uint32_t client_id = -1;

    //QEventLoop loop;

    // Conectás la señal del lobby con el cliente
    QObject::connect(&l, &Lobby::input_username, [&](const QString &name){
        username = name.toStdString();
        std::cout << "Username entered: " << username << std::endl;

        protocol.send_username(username);
        client_id = protocol.receive_id(); // wait for server response
        std::cout << "Received client ID: " << client_id << std::endl;

        //server send car market if username valid -> new ui where client choose car -> ui car upgrades -> return false
        l.hide();
        bool car_selected = car_selection();

        //car no selected, client close the game
        if (!car_selected) {
            //show enter username again
            l.show();
        }else{
            //car selected, start the game
            app.quit();
        }
        
        //if not, response with an error id and ask again

    });

    // exit signal
    QObject::connect(&l, &Lobby::game_close, [&]() {
        // client close the game
        client_id = -1;
        l.close();
        app.quit();
    });

    app.exec();

    return client_id;
}
