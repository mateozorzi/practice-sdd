#include "lobby.h"

#include <utility>


Lobby::Lobby(const ServerConfig& config): config(config) {}

std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> Lobby::create_new_game(
        const uint32_t client_id, const std::string& username, const uint8_t max_players) {

    std::lock_guard lock(mtx);

    uint32_t game_id = client_id;

    auto game = std::make_unique<Game>(game_id, max_players, config);

    std::cerr << "[" << LOBBY
              << "] "
                 "Creating new game with ID: "
              << game_id << " | Max players: " << (int)max_players
              << " | (Owner has the same id): " << client_id << std::endl;

    auto queues = game->add_player(client_id, username);
    games.emplace(game_id, std::move(game));


    return queues;
}

std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> Lobby::join_existing_game(
        const uint32_t client_id, const uint32_t game_id, const std::string& username) {

    std::lock_guard lock(mtx);

    auto it = games.find(game_id);

    if (it == games.end()) {

        std::cerr << "[" << LOBBY
                  << "] "
                     "Game "
                  << (int)game_id << " not found." << std::endl;
        static Queue<GameCommand> dummy_command;
        static Queue<WorldSnapshot> dummy_snapshot;
        return {dummy_command, dummy_snapshot};
    }

    Game& game = *(it->second);
    if (game.is_full()) {

        std::cerr << "[" << LOBBY << "] "
                  << "Game with ID: " << game_id << " is full." << std::endl;
        static Queue<GameCommand> dummy_command;
        static Queue<WorldSnapshot> dummy_snapshot;
        return {dummy_command, dummy_snapshot};
    }

    auto queues = game.add_player(client_id, username);

    return queues;
}

LobbyCatalogueDTO Lobby::list_games() {

    std::lock_guard lock(mtx);

    std::vector<LobbyDTO> lobby_dtos;

    for (const auto& [game_id, game_ptr]: games) lobby_dtos.emplace_back(game_id);

    return LobbyCatalogueDTO(std::move(lobby_dtos));
}

void Lobby::reap() {

    std::lock_guard lock(mtx);

    for (auto it = games.begin(); it != games.end();) {

        Game& game = *it->second;
        if (game.is_empty()) {
            std::cerr << "[" << LOBBY << "] "
                      << "Game with ID: " << (int)it->first << " is now empty, removing it." << std::endl;
            it = games.erase(it);
        } else {
            ++it;
        }
    }
}

void Lobby::remove_player(const uint32_t client_id, const uint32_t game_id) {

    std::lock_guard lock(mtx);

    const auto it = games.find(game_id);

    if (it == games.end())
        return;

    Game& game = *it->second;
    game.remove_player(client_id);
    std::cerr << "[" << LOBBY << "] "
              << "Player with ID: " << client_id << " removed from game with ID: " << (int)game_id
              << std::endl;
}
