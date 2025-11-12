#include "game.h"


Game::Game(const uint32_t game_id, const uint8_t max_players):
        max_players(max_players),
        game_id(game_id),
        command_queue(500),
        game_loop(game_monitor, command_queue, max_players) {}


bool Game::is_full() const { return game_loop.is_full(); }

bool Game::is_empty() const { return game_loop.is_empty(); }


std::pair<Queue<GameCommand>&, Queue<WorldSnapshot>&> Game::add_player(
        const uint32_t client_id, const std::string& username) {

    Queue<WorldSnapshot>& snapshot_queue = game_monitor.register_client(client_id);

    game_loop.add_player(client_id, username);

    if (game_loop.is_full())
        game_loop.start();

    return {command_queue, snapshot_queue};
}

void Game::remove_player(const uint32_t client_id) {

    game_monitor.unregister_client(client_id);
    game_loop.remove_player(client_id);
}


void Game::stop() {

    game_loop.stop();
    game_loop.join();

    command_queue.close();

    std::cout << "[GAME] Game " << game_id << " stopped." << std::endl;
}

Game::~Game() {

    try {

        Game::stop();
    } catch (const std::exception& e) {

        std::cerr << "[GAME] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
