#include "game_loop.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>


GameLoop::GameLoop(): command_queue() {}


void GameLoop::broadcast(const GameMessage& msg) {

    std::lock_guard<std::mutex> lock(mtx);

    for (auto* client_queue: client_queues) {

        try {

            client_queue->push(msg);
        } catch (const ClosedQueue&) {}
    }
}


void GameLoop::add_command(const GameCommand& cmd) { command_queue.push(cmd); }


void GameLoop::register_client(Queue<GameMessage>* client_queue) {

    std::lock_guard<std::mutex> lock(mtx);

    client_queues.push_back(client_queue);
}


void GameLoop::unregister_client(Queue<GameMessage>* client_queue) {

    std::lock_guard<std::mutex> lock(mtx);

    client_queues.erase(std::remove(client_queues.begin(), client_queues.end(), client_queue),
                        client_queues.end());
}


void GameLoop::run() {

    try {

        while (should_keep_running()) {

            GameCommand cmd;
            try {

                std::cout << "GAMELOOP RUNNING" << std::endl;
                while (command_queue.try_pop(cmd))
                    break;

            } catch (const ClosedQueue&) {

                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        }
    } catch (const ClosedQueue&) {}
}

void GameLoop::stop() {

    _keep_running = false;

    command_queue.close();

    std::lock_guard<std::mutex> lock(mtx);

    for (auto* q: client_queues) q->close();
}
