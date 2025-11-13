#include "receiver.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#include "../common/liberror.h"
#include "../common/movement_dto.h"


Receiver::Receiver(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t client_id,
                   Queue<GameCommand>& command_queue):
        server_protocol(server_protocol),
        lobby(lobby),
        client_id(client_id),
        command_queue(command_queue) {}

void Receiver::handle_get_car_catalogue() {

    std::vector<car_dto> cars = {// Name, Health, Handling, MaxSpeed, Acceleration
                                 {"Compact", 100, 90, 100, 80},    {"Sport", 80, 95, 150, 100},
                                 {"Convertible", 85, 90, 140, 95}, {"Sedan", 90, 85, 120, 75},
                                 {"Jeep", 150, 70, 85, 60},        {"Rally", 120, 80, 110, 70},
                                 {"Truck", 200, 50, 80, 40}};

    CarCatalogueDTO catalogue(std::move(cars));

    server_protocol.send_car_catalogue(catalogue);
}


void Receiver::run() {

    try {

        while (should_keep_running()) {

            GameCommandDTO game_command = server_protocol.receive_game_command();
            if (game_command.client_action == ClientAction::GET_CAR_CATALOGUE) {

                handle_get_car_catalogue();
                continue;
            } else if (game_command.client_action == ClientAction::DISCONNECT) {

                command_queue.push(GameCommand(client_id, std::move(game_command)));
                break;
            }

            command_queue.push(GameCommand(client_id, std::move(game_command)));
        }
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << EXCEPTION << ": " << e.what();
    }
}


void Receiver::stop() {

    if (_keep_running) {

        _is_alive = false;
        _keep_running = false;
    }
}

Receiver::~Receiver() {

    try {

        Receiver::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
