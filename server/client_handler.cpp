#include "client_handler.h"

#include <iostream>
#include <utility>


#include "../common/lobbies_catalogue_dto.h"
#include "../common/liberror.h"


ClientHandler::ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id):
        peer_socket(std::move(peer_socket)),
        server_protocol(this->peer_socket),
        lobby(lobby),
        client_id(client_id) {

}

void ClientHandler::launch_lobby() {

    server_protocol.receive_client_request();
    PlayerDTO player_dto = server_protocol.receive_username();
    server_protocol.send_client_id(client_id);

    while (is_in_lobby) {

        try {
            LobbyDTO lobby_dto = server_protocol.receive_lobby_info();
            switch (lobby_dto.action) {
                case LobbyAction::CREATE_LOBBY:
                    std::cout << "Creating new match with id: " << client_id << std::endl;
                    create_new_game(client_id, player_dto.username);
                    break;
                case LobbyAction::SEND_LOBBY_LIST:
                    std::cout << "Sent lobby catalogue to client with id: " << client_id << std::endl;
                    server_protocol.send_lobby_catalogue(lobby.list_games());
                    break;
                case LobbyAction::JOIN_LOBBY:
                    std::cout << "Client requested to join existing lobby." << std::endl;
                    join_existing_game(client_id, lobby_dto.lobby_id, player_dto.username);
                    break;
                // These below don't work for now...
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

void ClientHandler::exit_lobby(Queue<GameCommand>& command_queue, Queue<WorldSnapshot>& snapshot_queue) {

    is_in_lobby = false;
    Receiver receiver(server_protocol, lobby, client_id, command_queue);
    receiver.start();
    Sender sender(server_protocol, lobby, client_id, snapshot_queue);
    sender.run();

    sender.stop();
    receiver.stop();
    receiver.join();
}

void ClientHandler::create_new_game(const uint32_t client_id, const std::string& username) {

    auto [command_queue, snapshot_queue] = lobby.create_new_game(client_id, username);
    exit_lobby(command_queue, snapshot_queue);
}

void ClientHandler::join_existing_game(const uint32_t client_id, const uint32_t game_id, const std::string& username) {

    auto [command_queue, snapshot_queue] = lobby.join_existing_game(client_id, game_id, username);
    exit_lobby(command_queue, snapshot_queue);
}

void ClientHandler::run() {

    while (should_keep_running() && is_in_lobby) {
        try {


            launch_lobby();
        } catch (const LibError&) {

        } catch (const std::exception& e) {

            std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION << ": " << e.what();
        }
    }

    ClientHandler::stop();
}


void ClientHandler::stop() {

    try {

        server_protocol.stop();
    } catch (const LibError&) {}

    lobby.remove_player(client_id);
}


ClientHandler::~ClientHandler() {

    try {

        ClientHandler::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}

