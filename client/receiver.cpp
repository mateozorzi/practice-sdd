#include "receiver.h"

#include "../common/world_snapshot_dto.h"

Receiver::Receiver(ClientProtocol& protocol): protocol(protocol), car_positions_queue() {}

GameState Receiver::get_game_state() const { return game_state; }

Queue<WorldSnapshotDTO>& Receiver::get_car_positions_queue() {
    return car_positions_queue;
}

void Receiver::run() {
    while (should_keep_running()) {
        try {
            WorldSnapshotDTO car_position = protocol.receive_update_positions();
            car_positions_queue.push(car_position);
        } catch (const std::exception& e) {
            break;
        }
    }
}
