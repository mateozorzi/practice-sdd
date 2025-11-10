#ifndef LOBBY_H
#define LOBBY_H


#include <mutex>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>


#include "../common/queue.h"


#include "game.h"


// WIP -> Adding multiple games logic
class Lobby {


    std::mutex mtx;
    std::unordered_map<uint32_t, std::unique_ptr<Game>> games;
    uint32_t game_id = 0;


public:
    Lobby(const Lobby&) = delete;
    Lobby& operator=(const Lobby&) = delete;

    Lobby() = default;

    void reap();
    void remove_player(uint32_t game_id, uint32_t client_id);

    std::vector<uint32_t> list_games();

    Queue<GameCommand>& create_new_game(uint32_t client_id, const std::string& race_name);
    Queue<GameCommand>& join_existing_game(uint32_t client_id, uint32_t game_id);

};


#endif
