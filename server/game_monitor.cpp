#include "game_monitor.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "../common/constants.h"
#include "../common/leaderboard_dto.h"


GameMonitor::GameMonitor(const ServerConfig& config): config(config) {}

Queue<GameUpdateDTO>& GameMonitor::register_player(const uint32_t player_id) {

    std::lock_guard lock(mtx);

    auto queue_pointer = std::make_unique<Queue<GameUpdateDTO>>(
            static_cast<size_t>(config.server.snapshot_queue_capacity));
    Queue<GameUpdateDTO>& queue_reference = *queue_pointer;

    snapshot_queues[player_id] = std::move(queue_pointer);

    return queue_reference;
}

void GameMonitor::unregister_player(const uint32_t player_id) {

    std::lock_guard lock(mtx);

    const auto it = snapshot_queues.find(player_id);

    if (it != snapshot_queues.end()) {

        try {

            it->second->close();
        } catch (const std::exception&) {}
    }
}


void GameMonitor::broadcast(const GameUpdateDTO& info) {

    std::lock_guard lock(mtx);

    for (auto& [player_id, snapshot_queue]: snapshot_queues) {

        try {

            snapshot_queue->try_push(info);
        } catch (const std::exception&) {}
    }
}

void GameMonitor::send_update_to_player(const GameUpdateDTO& info, const uint32_t player_id) {

    std::lock_guard lock(mtx);

    const auto it = snapshot_queues.find(player_id);

    if (it == snapshot_queues.end())
        return;

    it->second->try_push(info);
}


void GameMonitor::remove_player(const uint32_t player_id) {

    std::lock_guard lock(mtx);

    const auto it = snapshot_queues.find(player_id);

    if (it == snapshot_queues.end())
        return;

    snapshot_queues.erase(it);
    std::cerr << "[" << GAME_MONITOR << "] "
              << "Removing player with ID: " << player_id << std::endl;
}


void GameMonitor::stop() {

    std::lock_guard lock(mtx);

    for (auto& [player_id, snapshot_queue]: snapshot_queues) {

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
