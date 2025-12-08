#include "client_handler.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include "../common/liberror.h"
#include "../common/lobbies_catalogue_dto.h"
#include "../common/race_catalogue_dto.h"


ClientHandler::ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id,
                             const ServerConfig& server_config):
        peer_socket(std::move(peer_socket)),
        server_protocol(this->peer_socket),
        lobby(lobby),
        server_config(server_config),
        player_id(client_id),
        selected_races() {}


void ClientHandler::launch_lobby() {

    while (is_in_lobby) {

        try {
            LobbyDTO lobby_dto = server_protocol.receive_lobby_info();
            switch (lobby_dto.action) {

                case LobbyAction::CHANGE_USERNAME:
                    update_player_username(lobby_dto.lobby_name);
                    break;
                case LobbyAction::CREATE_LOBBY:
                    create_new_game(lobby_dto.max_players, lobby_dto.lobby_name);
                    break;
                case LobbyAction::SEND_LOBBY_LIST:
                    list_active_games();
                    break;
                case LobbyAction::JOIN_LOBBY:
                    join_existing_game(lobby_dto.lobby_id);
                    break;
                case LobbyAction::LIST_RACES:
                    list_loaded_races();
                    break;
                case LobbyAction::SEND_RACE_SELECTION:
                    select_races(lobby_dto);
                    break;
                case LobbyAction::LEAVE_LOBBY:
                default:
                    ClientHandler::stop();
                    break;
            }
        } catch (const std::exception& e) {

            ClientHandler::stop();
        }
    }

    ClientHandler::stop();
}


void ClientHandler::update_player_username(const std::string& actual_username) {

    username = actual_username;

    server_protocol.send_player_id(player_id);
}


void ClientHandler::list_active_games() {

    lobby.reap();
    server_protocol.send_active_games(lobby.list_games());
}


void ClientHandler::send_car_catalogue() {

    std::vector<CarConfig> car_types = {

            {server_config.physics.compact},     {server_config.physics.sport},
            {server_config.physics.convertible}, {server_config.physics.sedan},
            {server_config.physics.jeep},        {server_config.physics.rally},
            {server_config.physics.truck}};

    std::vector<CarDTO> cars;
    cars.reserve(car_types.size());

    std::transform(car_types.begin(), car_types.end(), std::back_inserter(cars),
                   [](const auto& car_config) {
                       return CarDTO(
                               car_config.model, static_cast<uint16_t>(car_config.health),
                               static_cast<uint8_t>(
                                       (car_config.turn_torque + car_config.drift_factor) * 100),
                               static_cast<uint8_t>(car_config.max_speed),
                               static_cast<uint8_t>(car_config.acceleration));
                   });

    CarCatalogueDTO catalogue(std::move(cars));
    server_protocol.send_car_catalogue(catalogue);
}


void ClientHandler::exit_lobby_and_play(Queue<GameCommand>& command_queue,
                                        Queue<GameUpdateDTO>& snapshot_queue) {

    is_in_lobby = false;

    send_car_catalogue();
    Receiver receiver(server_protocol, lobby, player_id, command_queue);
    receiver.start();
    Sender sender(server_protocol, lobby, player_id, snapshot_queue);
    sender.run();

    sender.stop();
    receiver.stop();
    receiver.join();

    const uint32_t game_id = lobby.get_game_id(player_id);

    lobby.remove_player(player_id, game_id);

    if (_keep_running)
        is_in_lobby = true;
}

void ClientHandler::create_new_game(const uint8_t max_players, const std::string& game_name) {

    lobby.reap();

    if (max_players < static_cast<uint8_t>(server_config.server.min_players_per_game)) {

        LobbyDTO lobby_dto(LobbyAction::MIN_PLAYERS_ERROR, 0, "",
                           server_config.server.min_players_per_game);
        server_protocol.send_lobby_response(lobby_dto);
        return;
    } else if (max_players > static_cast<uint8_t>(server_config.server.max_players_per_game)) {

        LobbyDTO lobby_dto(LobbyAction::MAX_PLAYERS_ERROR, 0, "",
                           server_config.server.max_players_per_game);
        server_protocol.send_lobby_response(lobby_dto);
        return;
    }

    auto result =
            lobby.create_new_game(player_id, username, max_players, game_name, selected_races);
    if (!result.has_value()) {

        LobbyDTO lobby_dto(LobbyAction::CREATE_OR_JOIN_ERROR, 0, "", 0);
        server_protocol.send_lobby_response(lobby_dto);
        return;
    }

    auto [command_queue, snapshot_queue] = result.value();

    LobbyDTO lobby_dto(LobbyAction::CREATE_OR_JOIN_SUCCESS, 0, "", 0);
    server_protocol.send_lobby_response(lobby_dto);
    exit_lobby_and_play(command_queue, snapshot_queue);
}

void ClientHandler::join_existing_game(const uint32_t game_id) {

    lobby.reap();

    auto result = lobby.join_existing_game(player_id, game_id, username);

    if (!result.has_value()) {

        LobbyDTO lobby_dto(LobbyAction::CREATE_OR_JOIN_ERROR, 0, "", 0);
        server_protocol.send_lobby_response(lobby_dto);
        return;
    }

    auto [command_queue, snapshot_queue] = result.value();

    LobbyDTO lobby_dto(LobbyAction::CREATE_OR_JOIN_SUCCESS, 0, "", 0);
    server_protocol.send_lobby_response(lobby_dto);
    exit_lobby_and_play(command_queue, snapshot_queue);
}

void ClientHandler::list_loaded_races() {

    RaceCatalogueDTO races = lobby.list_races();
    server_protocol.send_races_catalogue(races);
}

void ClientHandler::select_races(const LobbyDTO& lobby_dto) {

    selected_races = lobby_dto.selected_races;
}

void ClientHandler::run() {

    while (should_keep_running()) {

        try {
            if (is_in_lobby)
                launch_lobby();
        } catch (const LibError&) {

        } catch (const std::exception& e) {

            std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION << ": " << e.what();
        }
    }
}


void ClientHandler::exit_game() {

    if (!is_in_lobby)
        server_protocol.send_disconnection();
}


void ClientHandler::stop() {

    if (_keep_running) {

        _keep_running = false;
        is_in_lobby = false;
        exit_game();
    }

    try {

        server_protocol.stop();
    } catch (const LibError&) {}
}


ClientHandler::~ClientHandler() {

    std::cerr << "[" << CLIENT_HANDLER << "] Stopping." << std::endl;
    try {

        ClientHandler::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
