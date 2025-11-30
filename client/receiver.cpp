#include "receiver.h"

#include <variant>

#include "../common/game_update_dto.h"
#include "../common/race_dto.h"
#include "../common/world_snapshot_dto.h"
#include "../common/liberror.h"

Receiver::Receiver(ClientProtocol& protocol): protocol(protocol), car_positions_queue() {}

GameState Receiver::get_game_state() const { return game_state; }

Queue<GameUpdateDTO>& Receiver::get_car_positions_queue() { return car_positions_queue; }

void Receiver::run() {

    while (should_keep_running()) {

        try {
            GameUpdateDTO car_position = protocol.receive_update();
            car_positions_queue.push(car_position);

            if (car_position.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

                _is_alive = false;
                std::cerr << "Gameloop action 'Return to Lobby' received. Exiting Receiver loop" << std::endl;
                break;
            }

        } catch (const std::exception& e) {
            break;
        }
    }
}
