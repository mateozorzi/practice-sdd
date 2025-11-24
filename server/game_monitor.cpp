#include "game_monitor.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "../common/constants.h"
#include "../common/leaderboard_dto.h"


GameMonitor::GameMonitor(const ServerConfig& config): config(config) {}

Queue<GameUpdateDTO>& GameMonitor::register_client(const uint32_t client_id) {

    std::lock_guard lock(mtx);

    auto queue_pointer = std::make_unique<Queue<GameUpdateDTO>>(
            static_cast<size_t>(config.server.snapshot_queue_capacity));
    Queue<GameUpdateDTO>& queue_reference = *queue_pointer;

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


void GameMonitor::broadcast(const GameUpdateDTO& info) {

    std::lock_guard lock(mtx);

    for (auto& [client_id, snapshot_queue]: snapshot_queues) {

        try {

            snapshot_queue->try_push(info);
        } catch (const std::exception&) {}
    }
}


void GameMonitor::remove_client(const uint32_t client_id) {

    std::lock_guard lock(mtx);

    const auto it = snapshot_queues.find(client_id);

    if (it != snapshot_queues.end())
        snapshot_queues.erase(it);
}


void GameMonitor::stop() {

    std::lock_guard lock(mtx);

    for (auto& [client_id, snapshot_queue]: snapshot_queues) {

        try {

            snapshot_queue->close();
        } catch (const std::exception&) {}
    }

    snapshot_queues.clear();
}


GameMonitor::~GameMonitor() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << GAME_MONITOR << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
