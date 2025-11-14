#ifndef GAME_MONITOR_H
#define GAME_MONITOR_H


#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../common/queue.h"

#include "game_command.h"
#include "server_config.h"
#include "world_snapshot.h"


class GameMonitor {


    std::mutex mtx;
    std::unordered_map<uint32_t, std::unique_ptr<Queue<WorldSnapshot>>> snapshot_queues;
    const ServerConfig& config;


public:
    GameMonitor(const GameMonitor&) = delete;
    GameMonitor& operator=(const GameMonitor&) = delete;

    explicit GameMonitor(const ServerConfig& config);

    Queue<WorldSnapshot>& register_client(uint32_t client_id);
    void unregister_client(uint32_t client_id);
    void broadcast(const WorldSnapshot& world_snapshot);
    void remove_client(uint32_t client_id);
    void stop();

    ~GameMonitor();
};


#endif
