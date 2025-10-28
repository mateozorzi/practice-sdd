#include <chrono>
#include <thread>

#include "receiver.h"

#include "common/liberror.h"


Receiver::Receiver(ServerProtocol& server_protocol, GameLoop& game_loop,
    const uint32_t client_id): server_protocol(server_protocol), game_loop(game_loop), client_id(client_id) {}


void Receiver::handle_car_movement(const ClientRequest& client_request) {

    const CarMovement car_movement = server_protocol.receive_car_movement();

    switch (car_movement) {

        case CarMovement::ACCELERATE:
            game_loop.add_command(GameCommand(client_id, client_request, CarMovement::ACCELERATE));
            break;
        case CarMovement::BRAKE:
            game_loop.add_command(GameCommand(client_id, client_request, CarMovement::BRAKE));
            break;
        case CarMovement::TURN_LEFT:
            game_loop.add_command(GameCommand(client_id, client_request, CarMovement::TURN_LEFT));
            break;
        case CarMovement::TURN_RIGHT:
            game_loop.add_command(GameCommand(client_id, client_request, CarMovement::TURN_RIGHT));
            break;
    }
}

void Receiver::run() {

    try {

        while (should_keep_running()) {

            const ClientRequest client_request = server_protocol.receive_client_request();
            if (client_request == ClientRequest::MOVE_CAR)
                handle_car_movement(client_request);
            else if (client_request == ClientRequest::NO_REQUEST)
                break;
        }
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << RECEIVER_EXCEPTION << ": " << e.what();
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

        std::cerr << "[" << RECEIVER << "] " << RECEIVER_EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
