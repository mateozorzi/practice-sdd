#include "sender.h"

#include <chrono>
#include <thread>

#include "../common/liberror.h"


Sender::Sender(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t client_id,
               Queue<std::variant<WorldSnapshotDTO, RaceDTO>>& snapshot_queue):
        server_protocol(server_protocol),
        lobby(lobby),
        client_id(client_id),
        snapshot_queue(snapshot_queue),
        keep_running(true) {}


void Sender::run() {

    try {

        while (keep_running) {

            try {

                auto variant_snapshot = snapshot_queue.pop();

                std::visit([this](const auto& snapshot) {
                    using T = std::decay_t<decltype(snapshot)>;
                    
                    if constexpr (std::is_same_v<T, WorldSnapshotDTO>) {
                        //WorldSnapshotDTO
                        //std::cout<< "[SENDER] Sending WorldSnapshotDTO to client: " << client_id << std::endl;
                        this->server_protocol.send_world_snapshot(snapshot);
                    }
                    else if constexpr (std::is_same_v<T, RaceDTO>) {
                        //RaceDTO
                        //std::cout<< "[SENDER] Sending RaceDTO to client: " << client_id << std::endl;
                        //this->server_protocol.send_race_update(snapshot); //not implemented
                    }else{
                        std::cout<< "[SENDER] ignoring snapshot: " << client_id << std::endl;
                    }
                    //else ignore
                }, variant_snapshot);

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
