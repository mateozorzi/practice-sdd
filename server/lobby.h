#ifndef LOBBY_H
#define LOBBY_H


#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../common/game_update_dto.h"
#include "../common/lobbies_catalogue_dto.h"
#include "../common/race_catalogue_dto.h"

#include "game.h"
#include "server_config.h"


constexpr std::string_view LOBBY = "LOBBY";


using GameQueues = std::pair<Queue<GameCommand>&, Queue<GameUpdateDTO>&>;


class Lobby {


    std::mutex mtx;
    std::unordered_map<uint32_t, std::unique_ptr<Game>> games;
    const ServerConfig& server_config;
    std::vector<RaceDTO> races;


public:
    Lobby(const Lobby&) = delete;
    Lobby& operator=(const Lobby&) = delete;

    explicit Lobby(const ServerConfig& server_config);

    std::optional<GameQueues> create_new_game(uint32_t player_id, const std::string& username,
                                              uint8_t max_players, const std::string& name,
                                              const RaceCatalogueDTO& selected_races);
    std::optional<GameQueues> join_existing_game(uint32_t player_id, uint32_t game_id,
                                                 const std::string& username);
    LobbyCatalogueDTO list_games();
    void reap();
    void remove_player(uint32_t player_id, uint32_t game_id);

    uint32_t get_game_id(uint32_t player_id);

    RaceCatalogueDTO list_races();

    void stop();

    ~Lobby();
};


#endif
