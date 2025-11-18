#ifndef GAME_H
#define GAME_H


#include <string>
#include <utility>

#include "../common/constants.h"
#include "../common/queue.h"

#include "game_command.h"
#include "game_loop.h"
#include "game_monitor.h"
#include "server_config.h"


class Game {


    const uint8_t max_players;
    const uint32_t game_id;
    const ServerConfig& config;
    std::string name;
    Queue<GameCommand> command_queue;
    GameMonitor game_monitor;
    GameLoop game_loop;


public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    explicit Game(uint32_t game_id, uint8_t max_players, const ServerConfig& config, std::string name, RaceCatalogueDTO& selected);

    bool is_full() const;
    bool is_empty() const;

    std::pair<Queue<GameCommand>&, Queue<std::variant<std::monostate, WorldSnapshotDTO, RaceDTO>>&> add_player(uint32_t client_id,
                                                                     const std::string& username);
    void remove_player(uint32_t client_id);

    const std::string get_name() const;
    uint8_t get_max_players() const;
    uint8_t get_active_players() const;

    bool has_started() const;

    void stop();

    ~Game();
};

#endif
