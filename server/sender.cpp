#include "sender.h"

#include <chrono>
#include <thread>

#include "../common/liberror.h"


Sender::Sender(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t client_id,
               Queue<WorldSnapshot>& snapshot_queue):
        server_protocol(server_protocol),
        lobby(lobby),
        client_id(client_id),
        snapshot_queue(snapshot_queue),
        keep_running(true) {}


void Sender::run() {

    try {

        while (keep_running) {

            try {

                const WorldSnapshot& world_snapshot = snapshot_queue.pop();
                server_protocol.send_world_snapshot(world_snapshot);

            } catch (const ClosedQueue&) {

                keep_running = false;
                break;
            }
        }
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << EXCEPTION << ": " << e.what();
    }
}

bool Sender::is_alive() { return keep_running; }

void Sender::stop() {

    if (keep_running) {

        keep_running = false;
    }
}


Sender::~Sender() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
