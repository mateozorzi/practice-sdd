#include "client_handler.h"

#include <iostream>
#include <utility>

#include "../common/liberror.h"


ClientHandler::ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id):
        peer_socket(std::move(peer_socket)),
        server_protocol(this->peer_socket),
        lobby(lobby),
        client_id(client_id),
        receiver(nullptr),
        sender(nullptr) {

    run_lobby_loop();
}


bool ClientHandler::is_alive() const { return receiver && sender && receiver->is_alive() && sender->is_alive(); }



void ClientHandler::stop() {

    try {

        server_protocol.stop();
    } catch (const LibError&) {}

    if (receiver && sender) {
        receiver->stop();
        sender->stop();

        receiver->join();
        sender->join();
    }

    //lobby.remove_player(client_id);
}


ClientHandler::~ClientHandler() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}




void ClientHandler::run_lobby_loop() {

    std::cout << "Lobby loop started for client id: " << client_id << std::endl;

    PlayerDTO player_dto = server_protocol.receive_username();
    std::cout << "Player connected with username: " << player_dto.username << " and client id: " << client_id << std::endl;
    server_protocol.send_client_id(client_id);

    while (is_in_lobby) {

        try {
            LobbyDTO lobby_dto = server_protocol.receive_lobby_info();

            switch (lobby_dto.action) {
                case LobbyAction::CREATE_LOBBY:
                    std::cout << "Creating new match!" << std::endl;
                    create_new_match(client_id);
                    std::cout << "Creating new match with id: " << client_id << std::endl;
                    break;
                case LobbyAction::SEND_LOBBY_LIST:
                    lobby.reap();
                    lobby.list_games();
                    std::cout << "Listing matches for player with client id: " << client_id << std::endl;
                    break;
                case LobbyAction::JOIN_LOBBY:
                    join_existing_match(client_id, lobby_dto.lobby_id);
                    std::cout << "Joining existing match!" << std::endl;
                    break;
                case LobbyAction::LEAVE_LOBBY:
                    player_dto = server_protocol.receive_username();
                    std::cout << "Username re-entered!" << std::endl;
                    break;
                default:
                    is_in_lobby = false;
                    break;  
            }
        }
        catch (const std::exception& e) {
        
            is_in_lobby = false;
        }
    }
}


void ClientHandler::exit_lobby() {

    is_in_lobby = false;
    sender->start();
    receiver->start();
}


void ClientHandler::create_new_match(const uint32_t client_id) {

    Queue<GameCommand>& game_queue = lobby.create_new_game(client_id);
    (void) game_queue;
    exit_lobby();
}


void ClientHandler::join_existing_match(const uint32_t client_id, const uint32_t game_id) {

    Queue<GameCommand>& game_queue = lobby.join_existing_game(client_id, game_id);
    (void) game_queue;
    exit_lobby();
}

