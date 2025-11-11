#ifndef LOBBY_H
#define LOBBY_H


#include <mutex>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>


#include "game.h"


class Lobby {


    std::mutex mtx;
    std::unordered_map<uint32_t, std::unique_ptr<Game>> games;


public:
    Lobby(const Lobby&) = delete;
    Lobby& operator=(const Lobby&) = delete;

    Lobby() = default;

    Queue<WorldSnapshot>& get_snapshot_queue(uint32_t game_id, uint32_t client_id);

    Queue<GameCommand>& create_new_game(uint32_t client_id);
    Queue<GameCommand>& join_existing_game(uint32_t client_id, uint32_t game_id);
    void list_games();
    void reap();
    void remove_player(uint32_t game_id, uint32_t client_id);


};


#endif
