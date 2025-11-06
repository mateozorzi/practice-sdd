#include "receiver.h"

#include <chrono>
#include <thread>

#include "../common/liberror.h"
#include "../common/movement_dto.h"


Receiver::Receiver(ServerProtocol& server_protocol, GameMonitor& game_monitor,
                   const uint32_t client_id):
        server_protocol(server_protocol), game_monitor(game_monitor), client_id(client_id) {}


void Receiver::handle_player_username(const ClientRequest& client_request) {

    const PlayerDTO& player_dto = server_protocol.receive_username();
    server_protocol.send_client_id(client_id);
    game_monitor.add_command(GameCommand(client_id, client_request, player_dto));
}

void Receiver::handle_get_car_catalogue() {

    std::vector<car_dto> cars = {
        // Name, Health, Handling, MaxSpeed, Acceleration
        {"Compact", 100, 90, 100, 80},
        {"Sport", 80, 95, 150, 100},
        {"Convertible", 85, 90, 140, 95},
        {"Sedan", 90, 85, 120, 75},
        {"Jeep", 150, 70, 85, 60},
        {"Rally", 120, 80, 110, 70},
        {"Truck", 200, 50, 80, 40}
    };

    CarCatalogueDTO catalogue(std::move(cars));

    server_protocol.send_car_catalogue(catalogue);
}

void Receiver::handle_car_movement(const ClientRequest& client_request) {

    const MovementDTO& movement_dto = server_protocol.receive_movement();
    game_monitor.add_command(GameCommand(client_id, client_request, movement_dto));
}

void Receiver::run() {

    try {

        while (should_keep_running()) {

            const ClientRequest client_request = server_protocol.receive_client_request();
            if (client_request == ClientRequest::SEND_USERNAME)
                handle_player_username(client_request);
            else if (client_request == ClientRequest::GET_CAR_CATALOGUE)
                handle_get_car_catalogue();
            else if (client_request == ClientRequest::MOVE_CAR)
                handle_car_movement(client_request);
            else if (client_request == ClientRequest::DISCONNECT)
                break;
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
