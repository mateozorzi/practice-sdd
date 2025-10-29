#include "receiver.h"

Receiver::Receiver(ClientProtocol& protocol): protocol(protocol), car_position_queue() {}

GameState Receiver::get_game_state() const { return game_state; }

Queue<CarPositionDTO>& Receiver::get_car_position_queue() { return car_position_queue; }

void Receiver::run() {
    while (should_keep_running()) {
        try {
            CarPositionDTO car_position = protocol.receive_car_position();
            car_position_queue.push(car_position);
        } catch (const std::exception& e) {
            break;
        }
    }
}
