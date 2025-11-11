#include "game_monitor.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "../common/constants.h"


Queue<WorldSnapshot>& GameMonitor::register_client(const uint32_t client_id) {

    std::lock_guard lock(mtx);

    auto queue_pointer = std::make_unique<Queue<WorldSnapshot>>(100);
    Queue<WorldSnapshot>& queue_reference = *queue_pointer;

    snapshot_queues[client_id] = std::move(queue_pointer);

    return queue_reference;
}

void GameMonitor::unregister_client(const uint32_t client_id) {

    std::lock_guard lock(mtx);

    const auto it = snapshot_queues.find(client_id);

    if (it != snapshot_queues.end()) {

        try {

            it->second->close();
        } catch (const std::exception&) {}
    }
}


void GameMonitor::broadcast(const WorldSnapshot& world_snapshot) {

    std::lock_guard lock(mtx);

    for (auto& [client_id, snapshot_queue]: snapshot_queues) {

        try {

            snapshot_queue->try_push(world_snapshot);
        } catch (const std::exception&) {}
    }
}


void GameMonitor::remove_client(const uint32_t client_id) {

    std::lock_guard lock(mtx);

    const auto it = snapshot_queues.find(client_id);

    if (it != snapshot_queues.end())
        snapshot_queues.erase(it);
}


/*void GameMonitor::register_command_queue(Queue<GameCommand>& command_queue) {

    command_queue_reference = std::ref(command_queue);
}

void GameMonitor::add_command(const GameCommand& command) const {

    if (command_queue_reference.has_value()) {

        try {

            command_queue_reference->get().push(command);
        } catch (const ClosedQueue&) {}
    }
}*/

GameMonitor::~GameMonitor() {

    try {

        snapshot_queues.clear();
    } catch (const std::exception& e) {

        std::cerr << "[" << GAME_MONITOR << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
