#include "lobby.h"

#include <utility>


std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> Lobby::create_new_game(
        const uint32_t client_id, const std::string& username, const uint8_t max_players) {

    std::lock_guard lock(mtx);

    uint32_t game_id = client_id;

    auto game = std::make_unique<Game>(game_id, max_players);

    std::cout << "[Lobby] Creating new game with ID " << game_id << " and max players "
              << (int)max_players << " (Owner has the same id): " << client_id << std::endl;

    auto queues = game->add_player(client_id, username);
    player_to_game[client_id] = game_id;
    games.emplace(game_id, std::move(game));


    return queues;
}

std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> Lobby::join_existing_game(
        const uint32_t client_id, const uint32_t game_id, const std::string& username) {

    std::lock_guard lock(mtx);

    auto it = games.find(game_id);

    if (it == games.end()) {
        std::cerr << "[Lobby] Error: game " << (int)game_id << " not found" << std::endl;
        static Queue<GameCommand> dummy_command;
        static Queue<WorldSnapshot> dummy_snapshot;
        return {dummy_command, dummy_snapshot};
    }

    Game& game = *(it->second);
    if (game.is_full()) {
        std::cerr << "[Lobby] Error: game " << game_id << " is full" << std::endl;
        static Queue<GameCommand> dummy_command;
        static Queue<WorldSnapshot> dummy_snapshot;
        return {dummy_command, dummy_snapshot};
    }

    auto queues = game.add_player(client_id, username);
    player_to_game[client_id] = game_id;

    std::cout << "[Lobby] Player " << client_id << " joined the game " << game_id << std::endl;

    return queues;
}

LobbyCatalogueDTO Lobby::list_games() {

    std::lock_guard lock(mtx);

    reap();

    std::vector<LobbyDTO> lobby_dtos;

    for (const auto& [game_id, game_ptr]: games) lobby_dtos.emplace_back(game_id);

    return LobbyCatalogueDTO(std::move(lobby_dtos));
}

void Lobby::reap() {

    // TODO: remove finished games
}

void Lobby::remove_player(const uint32_t client_id) {

    std::lock_guard lock(mtx);

    (void)client_id;

    std::cout << "Still not implemented Lobby::remove player!!" << std::endl;
}
