#include <chrono>
#include <thread>

#include "receiver.h"

#include "common/liberror.h"


Receiver::Receiver(ServerProtocol& server_protocol, GameLoop& game_loop,
    const uint32_t client_id): server_protocol(server_protocol), game_loop(game_loop), client_id(client_id) {}


void Receiver::run() {

    try {

        while (should_keep_running()) {

            const ClientRequest client_request = server_protocol.receive_client_request();
            if (client_request == ClientRequest::MOVE_CAR) {

                GameCommand command{client_id, 0x03};
                game_loop.add_command(command);
            }
        }
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << RECEIVER_EXCEPTION << ": " << e.what();
    }
}


void Receiver::stop() {

    if (_keep_running) {

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
