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
    std::unordered_map<uint32_t, uint32_t> player_to_game; 


public:
    Lobby(const Lobby&) = delete;
    Lobby& operator=(const Lobby&) = delete;

    Lobby() = default;

    std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> create_new_game(uint32_t client_id, const std::string& username);
    std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> join_existing_game(uint32_t client_id, uint32_t game_id, const std::string& username);
    void list_games();
    void reap();
    void remove_player(uint32_t client_id);


};


#endif
