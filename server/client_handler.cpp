#include "client_handler.h"

#include <iostream>
#include <utility>

#include "../common/liberror.h"
#include "../common/lobbies_catalogue_dto.h"
#include "../common/race_catalogue_dto.h"


ClientHandler::ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id,
                             const ServerConfig& server_config):
        peer_socket(std::move(peer_socket)),
        server_protocol(this->peer_socket),
        lobby(lobby),
        server_config(server_config),
        client_id(client_id),
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
                    create_new_game(client_id, lobby_dto.max_players, lobby_dto.lobby_name);
                    break;
                case LobbyAction::SEND_LOBBY_LIST:
                    list_active_games();
                    break;
                case LobbyAction::JOIN_LOBBY:
                    join_existing_game(client_id, lobby_dto.lobby_id);
                    break;
                case LobbyAction::LIST_RACES:
                    list_load_races();
                    break;
                case LobbyAction::SEND_RACE_SELECTION:
                    select_races(lobby_dto);
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

    server_protocol.send_client_id(client_id);
}


void ClientHandler::list_active_games() {

    lobby.reap();
    server_protocol.send_lobby_catalogue(lobby.list_games());
}


void ClientHandler::send_car_catalogue() {

    std::vector<std::pair<std::string, CarConfig>> car_types = {

        {std::string(CAR_COMPACT), server_config.physics.compact},
        {std::string(CAR_SPORT), server_config.physics.sport},
        {std::string(CAR_CONVERTIBLE), server_config.physics.convertible},
        {std::string(CAR_SEDAN), server_config.physics.sedan},
        {std::string(CAR_JEEP), server_config.physics.jeep},
        {std::string(CAR_RALLY), server_config.physics.rally},
        {std::string(CAR_TRUCK), server_config.physics.truck}
    };

    std::vector<CarDTO> cars;
    cars.reserve(car_types.size());

    for (const auto& [model, car_config] : car_types) {

        cars.emplace_back(
            model,
            static_cast<uint16_t>(car_config.health),
            static_cast<uint8_t>((car_config.turn_torque + car_config.drift_factor) * 100),
            static_cast<uint8_t>(car_config.max_speed),
            static_cast<uint8_t>(car_config.acceleration)
        );
    }

    CarCatalogueDTO catalogue(std::move(cars));
    server_protocol.send_car_catalogue(catalogue);
}


void ClientHandler::exit_lobby_and_play(Queue<GameCommand>& command_queue,
                                        Queue<GameUpdateDTO>& snapshot_queue) {

    is_in_lobby = false;
    send_car_catalogue();
    Receiver receiver(server_protocol, lobby, client_id, command_queue);
    receiver.start();
    Sender sender(server_protocol, lobby, client_id, snapshot_queue);
    sender.run();

    sender.stop();
    receiver.stop();
    receiver.join();
}

void ClientHandler::create_new_game(const uint32_t client_id, const uint8_t max_players,
                                    const std::string& lobby_name) {

    game_id = client_id;
    auto [command_queue, snapshot_queue] =
            lobby.create_new_game(client_id, username, max_players, lobby_name, selected_races);
    exit_lobby_and_play(command_queue, snapshot_queue);
}

void ClientHandler::join_existing_game(const uint32_t client_id, const uint32_t actual_game_id) {

    game_id = actual_game_id;

    auto [command_queue, snapshot_queue] = lobby.join_existing_game(client_id, game_id, username);
    exit_lobby_and_play(command_queue, snapshot_queue);
}

void ClientHandler::list_load_races() {

    RaceCatalogueDTO races = lobby.list_races();
    server_protocol.send_races_catalogue(races);
}

void ClientHandler::select_races(const LobbyDTO& lobby_dto) {

    selected_races = lobby_dto.selected_races;
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
