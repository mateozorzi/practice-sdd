#include "lobby.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

#include "../common/race_catalogue_dto.h"

#include "race_loader.h"


Lobby::Lobby(const ServerConfig& server_config):
        server_config(server_config),
        races(RaceLoader(server_config.world.races_folder).load_races()) {}


std::optional<GameQueues> Lobby::create_new_game(const uint32_t player_id,
                                                 const std::string& username,
                                                 const uint8_t max_players, const std::string& name,
                                                 const RaceCatalogueDTO& selected_races) {

    std::lock_guard lock(mtx);

    if (games.size() >= server_config.server.max_concurrent_games)
        return std::nullopt;

    uint32_t game_id = player_id;

    auto game = std::make_unique<Game>(max_players, server_config, name, selected_races);

    auto queues = game->add_player(player_id, username);
    games.emplace(game_id, std::move(game));

    return queues;
}

std::optional<GameQueues> Lobby::join_existing_game(const uint32_t player_id,
                                                    const uint32_t game_id,
                                                    const std::string& username) {

    std::lock_guard lock(mtx);

    auto it = games.find(game_id);

    if (it == games.end())
        return std::nullopt;

    Game& game = *(it->second);
    if (game.is_full())
        return std::nullopt;

    auto queues = game.add_player(player_id, username);

    return queues;
}

LobbyCatalogueDTO Lobby::list_games() {

    std::lock_guard lock(mtx);

    std::vector<LobbyDTO> lobby_dtos;

    for (const auto& [game_id, game_ptr]: games) {

        if (!game_ptr->has_started())
            lobby_dtos.emplace_back(LobbyAction::INFO_LOBBY, game_id, game_ptr->get_name(),
                                    game_ptr->get_max_players(), game_ptr->get_active_players());
    }

    return LobbyCatalogueDTO(std::move(lobby_dtos));
}

void Lobby::reap() {

    std::lock_guard lock(mtx);

    for (auto it = games.begin(); it != games.end();) {

        const Game& game = *it->second;

        if (game.is_empty())
            it = games.erase(it);
        else
            ++it;
    }
}

void Lobby::remove_player(const uint32_t player_id, const uint32_t game_id) {

    std::lock_guard lock(mtx);

    const auto it = games.find(game_id);

    if (it == games.end())
        return;

    Game& game = *it->second;

    game.remove_player(player_id);

    if (player_id == game_id && game.get_active_players() > 0) {

        auto node = games.extract(it);
        node.key() = game.get_next_player_id();
        games.insert(std::move(node));
    }
}


uint32_t Lobby::get_game_id(const uint32_t player_id) {

    std::lock_guard lock(mtx);

    for (const auto& [game_id, game_ptr]: games) {

        if (game_ptr->has_player(player_id))
            return game_id;
    }

    return 0;
}


RaceCatalogueDTO Lobby::list_races() {

    std::lock_guard lock(mtx);

    return RaceCatalogueDTO(std::move(races));
}


void Lobby::stop() {

    std::lock_guard lock(mtx);

    for (auto it = games.begin(); it != games.end();) it = games.erase(it);
}


Lobby::~Lobby() {

    std::cerr << "[" << LOBBY << "] Stopping." << std::endl;
    try {

        Lobby::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << LOBBY << "] " << EXCEPTION_IN_DESTRUCTOR << e.what();
    }
}
