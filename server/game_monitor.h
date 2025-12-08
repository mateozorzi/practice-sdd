#ifndef GAME_MONITOR_H
#define GAME_MONITOR_H


#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../common/game_update_dto.h"
#include "../common/leaderboard_dto.h"
#include "../common/queue.h"
#include "../common/race_dto.h"
#include "../common/world_snapshot_dto.h"

#include "game_command.h"
#include "server_config.h"
#include "server_constants.h"


class GameMonitor {


    std::mutex mtx;
    std::unordered_map<uint32_t, std::unique_ptr<Queue<GameUpdateDTO>>> snapshot_queues;
    const ServerConfig& config;


public:
    GameMonitor(const GameMonitor&) = delete;
    GameMonitor& operator=(const GameMonitor&) = delete;

    explicit GameMonitor(const ServerConfig& config);

    Queue<GameUpdateDTO>& register_player(uint32_t player_id);
    void unregister_player(uint32_t player_id);
    void unregister_all_players();

    void send_update_to_player(const GameUpdateDTO& info, uint32_t player_id);
    void broadcast(const GameUpdateDTO& info);

    void remove_player(uint32_t player_id);
    void stop();

    ~GameMonitor();
};


#endif
