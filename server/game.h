#ifndef GAME_H
#define GAME_H


#include "../common/queue.h"


#include "game_command.h"
#include "game_monitor.h"
#include "game_loop.h"


class Game {


    const uint32_t game_id;
    GameMonitor game_monitor;
    Queue<GameCommand> command_queue;
    GameLoop game_loop;


public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    explicit Game(uint32_t game_id);

    void stop();

    void add_player(uint32_t client_id);
    void remove_player(uint32_t client_id);

    bool is_full();
    bool is_empty();

};

#endif