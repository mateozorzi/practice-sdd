#include "game_loop.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>


GameLoop::GameLoop(): world(b2Vec2(0.0f, 0.0f)) {}


void GameLoop::broadcast(const CarPositionDTO& msg) {

    std::lock_guard<std::mutex> lock(mtx);

    for (auto* client_queue: client_queues) {

        try {

            client_queue->try_push(msg);
        } catch (const ClosedQueue&) {}
    }
}


void GameLoop::handle_car_movement(GameCommand& game_command) {

    const uint32_t client_id = game_command.client_id;
    const MovementDTO& movement_dto = game_command.movement_dto;

    auto [it, inserted] = cars.try_emplace(client_id, world);

    it->second.handle_movement(movement_dto);
}


void GameLoop::update_positions() {

    for (auto& [client_id, car] : cars) {

        car.update_position();

        CarPositionDTO car_position_dto = car.get_position_dto();
        broadcast(car_position_dto);
    }
}


void GameLoop::add_command(const GameCommand& cmd) { command_queue.push(cmd); }


void GameLoop::register_client(Queue<CarPositionDTO>* client_queue) {

    std::lock_guard<std::mutex> lock(mtx);

    client_queues.push_back(client_queue);
}


void GameLoop::unregister_client(Queue<CarPositionDTO>* client_queue) {

    std::lock_guard<std::mutex> lock(mtx);

    client_queue->close();

    client_queues.erase(std::remove(client_queues.begin(), client_queues.end(), client_queue),
                        client_queues.end());
}


void GameLoop::run() {

    try {

        while (should_keep_running()) {

            GameCommand game_command;
            try {

                while (command_queue.try_pop(game_command)) {
                    
                    if (game_command.client_request == ClientRequest::MOVE_CAR)
                        handle_car_movement(game_command);
                }

            } catch (const ClosedQueue&) {

                break;
            }

            update_positions();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    } catch (const ClosedQueue&) {}
}

void GameLoop::stop() {

    _keep_running = false;

    command_queue.close();
}
