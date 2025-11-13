#ifndef LOBBY_H
#define LOBBY_H


#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../common/lobbies_catalogue_dto.h"

#include "game.h"


class Lobby {


    std::mutex mtx;
    std::unordered_map<uint32_t, std::unique_ptr<Game>> games;


public:
    Lobby(const Lobby&) = delete;
    Lobby& operator=(const Lobby&) = delete;

    Lobby() = default;

    std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> create_new_game(
            uint32_t client_id, const std::string& username, uint8_t max_players);
    std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> join_existing_game(
            uint32_t client_id, uint32_t game_id, const std::string& username);
    LobbyCatalogueDTO list_games();
    void reap();
    void remove_player(uint32_t client_id, uint32_t game_id);

    ~Lobby() = default;
};


#endif
