#include "lobby.h"


Queue<WorldSnapshot>& Lobby::get_snapshot_queue(const uint32_t game_id, const uint32_t client_id) {

    std::lock_guard lock(mtx);

    auto it = games.find(game_id);

    if (it == games.end()) {

        std::cerr << "[Lobby] Error: game " << game_id << " not found" << std::endl;
        static Queue<WorldSnapshot> dummy;
        return dummy;
    }

    Game& game = *(it->second);
    return game.add_player(client_id);
}


Queue<GameCommand>& Lobby::create_new_game(const uint32_t client_id) {

    std::lock_guard lock(mtx);

    uint32_t game_id = client_id;

    auto game = std::make_unique<Game>(game_id);

    std::cout << "[Lobby] Creating new game with ID " << game_id
              << " (Owner has the same id): " << client_id << std::endl;

    games.emplace(game_id, std::move(game));

    return game->get_command_queue();
}

Queue<GameCommand>& Lobby::join_existing_game(const uint32_t client_id, const uint32_t game_id) {

    std::lock_guard lock(mtx);

    auto it = games.find(game_id);

    if (it == games.end()) {

        std::cerr << "[Lobby] Error: game " << game_id << " not found" << std::endl;
        static Queue<GameCommand> dummy;
        return dummy;
    }
    
    Game& game = *(it->second);
    if (game.is_full()) {

        std::cerr << "[Lobby] Error: partida " << game_id << " está llena\n";
        static Queue<GameCommand> dummy;
        return dummy;
    }

    game.add_player(client_id);
    std::cout << "[Lobby] Jugador " << client_id << " se unió a la partida " << game_id << "\n";

    return game.get_command_queue();
}

void Lobby::reap() {
    // TODO: eliminar partidas terminadas
}

void Lobby::list_games() {
    // TODO: imprimir o enviar lista de partidas activas
}