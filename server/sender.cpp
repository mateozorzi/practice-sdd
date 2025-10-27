#include <chrono>
#include <thread>

#include "sender.h"

#include "common/liberror.h"


Sender::Sender(ServerProtocol& server_protocol, GameLoop& game_loop, const uint32_t client_id):
    server_protocol(server_protocol), game_loop(game_loop), client_id(client_id) {

    game_loop.register_client(&client_message_queue);
}


void Sender::run() {

    try {

        while (should_keep_running()) {

            try {

                const CarPositionDTO& car_position_dto = client_message_queue.pop();
                server_protocol.send_car_position(car_position_dto);

            } catch (const ClosedQueue&) {

                break;
            }
        }
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << SENDER_EXCEPTION << ": " << e.what();
    }
}

void Sender::stop() {

    if (_keep_running) {

        _keep_running = false;
    }
}


Sender::~Sender() {

    try {

        Sender::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << SENDER_EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
