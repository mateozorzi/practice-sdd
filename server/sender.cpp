#include "sender.h"

#include <chrono>
#include <thread>

#include "../common/liberror.h"


Sender::Sender(ServerProtocol& server_protocol, GameMonitor& game_monitor,
               const uint32_t client_id):
        server_protocol(server_protocol),
        game_monitor(game_monitor),
        client_id(client_id),
        client_message_queue(game_monitor.register_client(client_id)) {}


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

        std::cerr << "[" << SENDER << "] " << EXCEPTION << ": " << e.what();
    }
}

void Sender::stop() {

    if (_keep_running) {

        _is_alive = false;
        _keep_running = false;
        game_monitor.unregister_client(client_id);
    }
}


Sender::~Sender() {

    try {

        Sender::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
