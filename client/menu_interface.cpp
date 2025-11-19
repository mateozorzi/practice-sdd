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
#include "Lobby/waitingroom.h"
#include "lobbyreceiver.h"
#include "Lobby/raceselector.h"
#include <QString>
#include <QEventLoop>
#include "../common/car_dto.h"
#include "../common/car_catalogue_dto.h"
#include "../common/player_dto.h"
#include "../common/lobby_dto.h"
#include "../common/lobbies_catalogue_dto.h"

MenuInterface::MenuInterface(ClientProtocol& protocol) : protocol(protocol) {}


bool MenuInterface::car_selection(PlayerDTO& player){
    //server send market of cars
    std::cout<<"Requesting car catalogue from server..."<<std::endl;
    protocol.ask_for_catalogue();
    CarCatalogueDTO catalogue = protocol.receive_car_catalogue();

    std::cout << "Received car catalogue of size: " << static_cast<int>(catalogue.size) << std::endl;
    SelectCars sc(catalogue);
    sc.show();

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

bool MenuInterface::select_races(){
    protocol.ask_for_race_catalogue();
    RaceCatalogueDTO races = protocol.receive_race_catalogue();
    
    RaceSelector sr(races);
    sr.show();

    bool closed = false;

    QEventLoop loop;

    //confirm signal
    QObject::connect(&sr, &RaceSelector::confirm_races, [&](std::vector<RaceDTO> races) {
        //send to server the races selected
        std::cout << "Races selected for the lobby: " << races.size() << std::endl;
        protocol.send_races_selection(races);
        sr.close();
        loop.quit();
    });

    //close menu signal
    QObject::connect(&sr, &RaceSelector::close_menu, [&]() {
        closed = true;
        loop.quit();
    });

    loop.exec();

    return closed;


}

bool MenuInterface::create_lobby(PlayerDTO& player) {
    //create lobby ui
    CreateLobby cl;
    cl.show();

    bool closed = false;

    QEventLoop loop;

    //create lobby signal
    QObject::connect(&cl, &CreateLobby::create_lobby, [&]() {
        //save info of ther lobby, and send to server
        std::string lobby_name = cl.get_lobby_name().toStdString();
        int max_players = cl.get_number_players();
        std::cout <<player.username<< " creating lobby: " << lobby_name << " with max players: " << max_players << std::endl;
        LobbyDTO lobby(LobbyAction::CREATE_LOBBY, player.id, lobby_name, max_players);
        cl.close();
        
        //select races ui
        closed = select_races();

        if (closed){
            loop.quit();
            return;
        }
        
        protocol.send_create_lobby(lobby);

        //closed = waiting_room(true, lobby);

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
    //client ask for available lobbies
    LobbyDTO lobby_request(LobbyAction::SEND_LOBBY_LIST);
    protocol.ask_for_lobbies(lobby_request);
    LobbyCatalogueDTO lobbies = protocol.receive_lobby_catalogue();

    std::cout<<"Received lobby catalogue of size: " << (int)lobbies.size << std::endl;
    
    
    // join lobby ui
    JoinLobby jl(lobbies);
    jl.show();

    bool closed = false;


    //server sends available lobbies
    QEventLoop loop;

    //join lobby signal
    QObject::connect(&jl, &JoinLobby::join_lobby, [&](const LobbyDTO& lobby) {
        //client select a lobby and send it to server
        std::cout << "Sent join lobby request for lobby ID: " << lobby.lobby_id << std::endl;

        //wait for the server to confirm join
        LobbyDTO lobby_req(LobbyAction::JOIN_LOBBY, lobby.lobby_id);
        protocol.send_action_lobby(lobby_req);
        //wait for server response
        //LobbyDTO lobby_info = protocol.receive_lobby_info();

        std::cout << player.username <<" joined lobby ID: " << lobby.lobby_id << std::endl;
        //jl.hide();
        //closed = waiting_room(false, lobby);

        if (closed) {
            jl.show();
        }else{
            jl.close();
            loop.quit();
        }

        /*if (lobby_info.action == LobbyAction::JOIN_LOBBY) {
            //lobby accepted
            std::cout << player.username <<" joined lobby ID: " << lobby.lobby_id << std::endl;
            jl.hide();
            closed = waiting_room(false, lobby);

            if (closed) {
                jl.show();
            }else{
                jl.close();
                loop.quit();
            }

        }else {
            //lobby denied
            std::cout << player.username <<" join lobby ID: " << lobby.lobby_id << " denied." << std::endl;
            jl.fill_table(lobbies);
            jl.join_denied();
        }*/


    });

    //refresh lobbies signal
    QObject::connect(&jl, &JoinLobby::refresh, [&]() {
        //request lobbies to server and update ui

        std::cout <<player.username <<" refreshing available lobbies..." << std::endl;

        //server sends available lobbies
        LobbyDTO lobby_request(LobbyAction::SEND_LOBBY_LIST);
        protocol.ask_for_lobbies(lobby_request);
        LobbyCatalogueDTO lobbies = protocol.receive_lobby_catalogue();

        //refresh table
        jl.fill_table(lobbies);
        
    });

    //close menu signal
    QObject::connect(&jl, &JoinLobby::close_menu, [&]() {
        closed = true;
        loop.quit();
    });
    
    loop.exec();

    return closed;

}

bool MenuInterface::waiting_room(const bool is_owner, const LobbyDTO& lobby) {
    WaitingRoom w(is_owner, lobby);
    w.show();

    bool closed = false;

    LobbyReceiver lobbyReceiver(protocol, &w);

    QEventLoop loop;

    //lobby started signal from lobbyReceiver
    QObject::connect(&lobbyReceiver, &LobbyReceiver::lobbyStarted, [&](){        
        std::cout<< "received lobby started signal for lobby ID: " << lobby.lobby_id << std::endl;
        w.close();
        loop.quit();
    });

    //lobby deleted signal from lobbyReceiver
    QObject::connect(&lobbyReceiver, &LobbyReceiver::lobbyDeleted, [&](){
        std::cout<< "received lobby deleted signal for lobby ID: " << lobby.lobby_id << std::endl;
        closed = true;
        loop.quit();
    });

    lobbyReceiver.start();

    //start lobby signal
    QObject::connect(&w, &WaitingRoom::start_lobby, [&]() {
        std::cout<<"emitting start lobby for lobby ID: " << lobby.lobby_id << std::endl;
        LobbyDTO start(LobbyAction::START_LOBBY, lobby.lobby_id);
        protocol.send_action_lobby(start);
        
    });

    //exit signal
    QObject::connect(&w, &WaitingRoom::close_menu, [&]() {
        std::cout<<"emiting leave lobby for lobby ID: " << lobby.lobby_id << std::endl;
        QObject::disconnect(&lobbyReceiver, &LobbyReceiver::lobbyStarted, nullptr, nullptr);
        QObject::disconnect(&lobbyReceiver, &LobbyReceiver::lobbyDeleted, nullptr, nullptr);
        
        LobbyDTO leave(LobbyAction::LEAVE_LOBBY, lobby.lobby_id);
        protocol.send_action_lobby(leave);

        closed = true;
        w.close();
        loop.quit();
        /*if (is_owner){
            std::cout << "Owner leaving, waits for server signal to delete lobby ID: " << lobby.lobby_id << std::endl;
            LobbyDTO leave(LobbyAction::LEAVE_LOBBY, lobby.lobby_id);
            protocol.send_action_lobby(leave);
        }else{
            std::cout<<"Non-owner leaving lobby ID: " << lobby.lobby_id << std::endl;
            QObject::disconnect(&lobbyReceiver, &LobbyReceiver::lobbyStarted, nullptr, nullptr);
            QObject::disconnect(&lobbyReceiver, &LobbyReceiver::lobbyDeleted, nullptr, nullptr);
            
            LobbyDTO leave(LobbyAction::LEAVE_LOBBY, lobby.lobby_id);
            protocol.send_action_lobby(leave);
    
            closed = true;
            loop.quit();
        }*/
        
    });


    loop.exec();

    if (lobbyReceiver.is_alive()){
        lobbyReceiver.stop();
        lobbyReceiver.join();
    }

    std::cout << "Is thread LobbyReceiver alive? " << lobbyReceiver.is_alive() << std::endl;

    return closed;
}

bool MenuInterface::lobby_create(PlayerDTO& player) {
    creategame cg;
    cg.show();

    bool closed = false;
    bool owner = false;

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

            owner = true;

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
    l.show();

    PlayerDTO player("", -1, car_dto(), -1,false);

    std::string username;
    uint32_t client_id = -1;

    bool menu_closed = false;

    // Conectás la señal del lobby con el cliente
    QObject::connect(&l, &Lobby::input_username, [&](const QString &name){
        username = name.toStdString();

        protocol.change_username(LobbyDTO(LobbyAction::CHANGE_USERNAME, username));            
        client_id = protocol.receive_id(); // wait for server response
        player.username = username;
        player.id = client_id;
        std::cout << "Username entered: " << player.username << std::endl;
        std::cout << "Received client ID: " << player.id << std::endl;

        
        //server send car market if username valid -> enter lobby (create or join)
        l.hide();
        menu_closed = lobby_create(player);
        std::cout << "Menu closed: " << menu_closed << std::endl;
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
        protocol.send_exit(LobbyDTO(LobbyAction::EXIT_GAME));
        player.id = -1;

        l.close();
        app.quit();
    });

    app.exec();

    return player;
}
