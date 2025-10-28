#include "game_loop.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>


GameLoop::GameLoop(): command_queue() {}


void GameLoop::handle_car_movement(GameCommand& game_command) {

    const CarMovement car_movement = game_command.car_movement;
    CarPositionDTO car_position_dto;

    switch (car_movement) {


        case CarMovement::ACCELERATE:
            car_position_dto = CarPositionDTO(0, 0, 0);
            break;
        case CarMovement::BRAKE:
            car_position_dto = CarPositionDTO(0, 10, 0);
            break;
        case CarMovement::TURN_LEFT:
            car_position_dto = CarPositionDTO(0, 0, 0);
            break;
        case CarMovement::TURN_RIGHT:
            car_position_dto = CarPositionDTO(10, 0, 0);
            break;
    }
     

    broadcast(car_position_dto);

    std::cout << "GAMELOOP Broadcasting hardcoded position: " << std::endl;
    std::cout << "Car x: " << car_position_dto.x_position << std::endl;
    std::cout << "Car y: " << car_position_dto.y_position << std::endl;
    std::cout << "Car angle: " << car_position_dto.angle << std::endl;
}


void GameLoop::broadcast(const CarPositionDTO& msg) {

    std::lock_guard<std::mutex> lock(mtx);

    for (auto* client_queue: client_queues) {

        try {

            client_queue->try_push(msg);
        } catch (const ClosedQueue&) {}
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

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    } catch (const ClosedQueue&) {}
}

void GameLoop::stop() {

    _keep_running = false;

    command_queue.close();
}
