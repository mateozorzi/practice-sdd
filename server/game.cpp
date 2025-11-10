#include "game.h"


Game::Game(const uint32_t game_id): game_id(game_id), game_loop(game_monitor, command_queue) {}



void Game::stop() {

    game_loop.stop();
    game_loop.join();

    std::cout << "[GAME] Game " << game_id << " stopped.\n";
}


bool Game::is_full() {

    return game_loop.is_full();
}


bool Game::is_empty() {

    return game_loop.is_empty();
}


void Game::add_player(const uint32_t client_id) {

    game_monitor.register_client(client_id);
    game_loop.add_player(client_id);

    if (game_loop.is_full())
        game_loop.start();
}

void Game::remove_player(const uint32_t client_id) {

    game_monitor.unregister_client(client_id);
    game_loop.remove_player(client_id);

}