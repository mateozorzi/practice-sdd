#include "game_loop.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>


GameLoop::GameLoop(): command_queue() {}


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
                        break;
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
