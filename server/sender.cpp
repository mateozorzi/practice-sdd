#include "sender.h"

#include <chrono>
#include <thread>

#include "../common/liberror.h"
#include "../common/game_update_dto.h"


Sender::Sender(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t client_id,
               Queue<GameUpdateDTO>& snapshot_queue):
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
                
                if (variant_snapshot.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS){
                    //WorldSnapshotDTO
                    //std::cout<< "[SENDER] Sending WorldSnapshotDTO to client: " << client_id << std::endl;
                    this->server_protocol.send_world_snapshot(variant_snapshot);
                }else if (variant_snapshot.gameloop_action == GameLoopAction::SEND_RACE){
                    //RaceDTO
                    std::cout<< "[SENDER] Sending RaceDTO to client: " << client_id << std::endl;
                    this->server_protocol.send_race_update(variant_snapshot);
                }else if (variant_snapshot.gameloop_action == GameLoopAction::SEND_LEADERBOARD){
                    this->server_protocol.send_leaderboard_update(variant_snapshot);
                }else {
                    std::cout<< "[SENDER] ignoring snapshot: " << client_id << std::endl;
                }

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
