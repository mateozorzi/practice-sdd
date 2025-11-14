#include "client_handler.h"

#include <iostream>
#include <utility>

#include "../common/liberror.h"
#include "../common/lobbies_catalogue_dto.h"


ClientHandler::ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id,
                             const ServerConfig& config):
        peer_socket(std::move(peer_socket)),
        server_protocol(this->peer_socket),
        lobby(lobby),
        config(config),
        client_id(client_id) {}


void ClientHandler::launch_lobby() {

    while (is_in_lobby) {

        try {

            LobbyDTO lobby_dto = server_protocol.receive_lobby_info();
            switch (lobby_dto.action) {

                case LobbyAction::CHANGE_USERNAME:
                    update_player_username(lobby_dto.lobby_name);
                    break;
                case LobbyAction::CREATE_LOBBY:
                    create_new_game(client_id, lobby_dto.max_players);
                    break;
                case LobbyAction::SEND_LOBBY_LIST:
                    list_active_games();
                    break;
                case LobbyAction::JOIN_LOBBY:
                    join_existing_game(client_id, lobby_dto.lobby_id);
                    break;
                case LobbyAction::LEAVE_LOBBY:
                default:
                    is_in_lobby = false;
                    break;
            }
        } catch (const std::exception& e) {

            is_in_lobby = false;
        }
    }

    is_in_lobby = false;
}


void ClientHandler::update_player_username(const std::string& actual_username) {

    username = actual_username;

    std::cerr << "[" << CLIENT_HANDLER << "] " << "Updated username for client with ID: " << client_id
        << " to: " << username << std::endl;
    
    server_protocol.send_client_id(client_id);
}


void ClientHandler::list_active_games() {

    lobby.reap();
    server_protocol.send_lobby_catalogue(lobby.list_games());
    std::cerr << "[" << CLIENT_HANDLER << "] " << "Sent lobby catalogue to client with ID: " << client_id
        << std::endl;
}

void ClientHandler::exit_lobby_and_play(Queue<GameCommand>& command_queue,
                               Queue<WorldSnapshotDTO>& snapshot_queue) {

    is_in_lobby = false;
    Receiver receiver(server_protocol, lobby, client_id, command_queue, config);
    receiver.start();
    Sender sender(server_protocol, lobby, client_id, snapshot_queue);
    sender.run();

    sender.stop();
    receiver.stop();
    receiver.join();
}

void ClientHandler::create_new_game(const uint32_t client_id, const uint8_t max_players) {

    std::cerr << "[" << CLIENT_HANDLER << "] " << "Creating new game with ID: " << client_id
            << " | Max players: " << max_players << std::endl;
    game_id = client_id;
    auto [command_queue, snapshot_queue] = lobby.create_new_game(client_id, username, max_players);
    exit_lobby_and_play(command_queue, snapshot_queue);
}

void ClientHandler::join_existing_game(const uint32_t client_id, const uint32_t actual_game_id) {

    std::cerr << "[" << CLIENT_HANDLER << "] " << "Client with ID: " << client_id << " requested to join existing lobby." << std::endl;
    game_id = actual_game_id;

    auto [command_queue, snapshot_queue] = lobby.join_existing_game(client_id, game_id, username);
    exit_lobby_and_play(command_queue, snapshot_queue);
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

    lobby.remove_player(client_id, game_id);
}


ClientHandler::~ClientHandler() {

    try {

        ClientHandler::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
