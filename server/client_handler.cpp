#include "client_handler.h"

#include <iostream>
#include <utility>

#include "../common/liberror.h"


ClientHandler::ClientHandler(Socket&& peer_socket, GameMonitor& game_monitor, uint32_t client_id):
        peer_socket(std::move(peer_socket)),
        server_protocol(this->peer_socket),
        game_monitor(game_monitor),
        client_id(client_id),
        receiver(server_protocol, game_monitor, client_id),
        sender(server_protocol, game_monitor, client_id) {

    receiver.start();
    sender.start();
}

/*

WIP -> Adding multiple games logic

ClientHandler::ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id):
        peer_socket(std::move(peer_socket)),
        server_protocol(this->peer_socket),
        lobby(lobby),
        client_id(client_id),
        receiver(server_protocol, lobby, client_id),
        sender(server_protocol, lobby, client_id) {

    run_lobby_loop();
}*/


bool ClientHandler::is_alive() const { return receiver.is_alive() && sender.is_alive(); }


void ClientHandler::stop() {

    try {

        server_protocol.stop();
    } catch (const LibError&) {}

    receiver.stop();
    sender.stop();

    receiver.join();
    sender.join();

    game_monitor.remove_client(client_id);
}


/*
void ClientHandler::stop() {

    try {

        server_protocol.stop();
    } catch (const LibError&) {}

    receiver.stop();
    sender.stop();

    receiver.join();
    sender.join();

    lobby.remove_player(client_id);
}
*/

ClientHandler::~ClientHandler() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}


/*

void ClientHandler::run_lobby_loop() {

    const PlayerDTO& player_dto = server_protocol.receive_username();
    server_protocol.send_client_id(client_id);

    while (is_in_lobby) {

        try {
            LobbyDTO lobby_dto = server_protocol.receive_lobby_dto();

            if (lobby_dto.create_new_match) {

                create_new_match();
                std::cout << "Creating new match with id: " << client_id << std::endl;
            } else if (lobby_dto.list_matches) {

                lobby.reap();
                lobby.list_matches();
                std::cout << "Listing matches for player with client id: " << client_id << std::endl;
            } else if (lobby_dto.join_match) {

                join_existing_match();
                std::cout << "Joining existing match!" << std::endl;
            } else if (lobby_dto.ask_username_again) {
            
                player_dto = server_protocol.receive_username();
                std::cout << "Username re-entered!" << std::endl;
            } else {
             
                break;
            }
        }
    } catch (const std::exception& e) {
     
        is_in_lobby = false();
    }
}


void ClientHandler::extit_lobby() {

    is_in_lobby = false;
    sender.start();
    receiver.start();
}


void ClientHandler::create_new_match() {

    // Queue<GameCommand>& game_queue = lobby.create_new_game(client_id, race_name);
    exit_lobby();
}


void ClientHandler::join_existing_match() {

    // Queue<GameCommand>& game_queue = lobby.join_existing_game(client_id, game_id);
    exit_lobby();
}*/

