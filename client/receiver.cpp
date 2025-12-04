#include "receiver.h"

#include <iostream>
#include <variant>

#include "../common/game_update_dto.h"
#include "../common/liberror.h"
#include "../common/race_dto.h"
#include "../common/world_snapshot_dto.h"

Receiver::Receiver(ClientProtocol& protocol): protocol(protocol), car_positions_queue() {}

Queue<GameUpdateDTO>& Receiver::get_car_positions_queue() { return car_positions_queue; }


bool Receiver::is_server_connected() const { return server_connected; }


void Receiver::run() {

    try {

        while (should_keep_running()) {

            GameUpdateDTO car_position = protocol.receive_update();
            car_positions_queue.push(car_position);

            if (car_position.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

                _is_alive = false;
                break;
            } else if (car_position.gameloop_action == GameLoopAction::DISCONNECT) {

                _is_alive = false;
                server_connected = false;
                break;
            }
        }
    } catch (const LibError& e) {

    } catch (const std::exception& e) {

        std::cerr << "["
                  << "RECEIVER"
                  << "] " << EXCEPTION << ": " << e.what();
    }
}

void Receiver::stop() {

    if (_keep_running)
        _keep_running = false;
}


Receiver::~Receiver() {

    try {

        Receiver::stop();
    } catch (const std::exception& e) {

        std::cerr << "["
                  << "RECEIVER"
                  << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
