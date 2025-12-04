#include "sender.h"

#include <chrono>
#include <thread>

#include "../common/game_update_dto.h"
#include "../common/liberror.h"


Sender::Sender(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t player_id,
               Queue<GameUpdateDTO>& snapshot_queue):
        server_protocol(server_protocol),
        lobby(lobby),
        player_id(player_id),
        snapshot_queue(snapshot_queue),
        keep_running(true) {}


void Sender::run() {

    try {

        while (keep_running) {

            try {

                auto variant_snapshot = snapshot_queue.pop();

                if (variant_snapshot.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS)
                    this->server_protocol.send_world_snapshot(variant_snapshot);
                else if (variant_snapshot.gameloop_action == GameLoopAction::SEND_RACE)

                    this->server_protocol.send_race_update(variant_snapshot);
                else if (variant_snapshot.gameloop_action == GameLoopAction::SEND_LEADERBOARD)
                    this->server_protocol.send_leaderboard_update(variant_snapshot);
                else if (variant_snapshot.gameloop_action == GameLoopAction::START_SHOP)
                    this->server_protocol.send_action_info(variant_snapshot);
                else if (std::holds_alternative<std::monostate>(variant_snapshot.data))
                    this->server_protocol.send_game_action(variant_snapshot);
                else if (variant_snapshot.gameloop_action == GameLoopAction::COUNTDOWN_TIME)
                    this->server_protocol.send_action_info(variant_snapshot);
                else if (variant_snapshot.gameloop_action == GameLoopAction::SEND_UPGRADES_REMAINING)
                    this->server_protocol.send_action_info(variant_snapshot);
                else if (variant_snapshot.gameloop_action == GameLoopAction::SEND_UPGRADE_SHOP_TIME_REMAINING)
                    this->server_protocol.send_action_info(variant_snapshot);

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

    if (keep_running)
        keep_running = false;
}


Sender::~Sender() {

    std::cerr << "[" << SENDER << "] Stopping." << std::endl;
    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
