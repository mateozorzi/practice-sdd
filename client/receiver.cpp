#include "receiver.h"

#include "../common/world_snapshot_dto.h"
#include "../common/race_dto.h"
#include <variant>

Receiver::Receiver(ClientProtocol& protocol): protocol(protocol), car_positions_queue() {}

GameState Receiver::get_game_state() const { return game_state; }

Queue<std::variant<std::monostate, WorldSnapshotDTO, RaceDTO>>& Receiver::get_car_positions_queue() {
    return car_positions_queue;
}

void Receiver::run() {
    while (should_keep_running()) {
        try {
            std::variant<std::monostate, WorldSnapshotDTO, RaceDTO> car_position = protocol.receive_update();
            car_positions_queue.push(car_position);
        } catch (const std::exception& e) {
            break;
        }
    }
}
