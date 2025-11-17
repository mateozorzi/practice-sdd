#include "../common/constants.h"


#include "game.h"
#include "../common/race_catalogue_dto.h"


Game::Game(const uint32_t game_id, const uint8_t max_players, const ServerConfig& config, std::string name, RaceCatalogueDTO& selected):
        max_players(max_players),
        game_id(game_id),
        config(config),
        name(std::move(name)),
        command_queue(static_cast<size_t>(config.app.server.command_queue_capacity)),
        game_monitor(config),
        game_loop(game_monitor, command_queue, max_players, config, selected) {}


bool Game::is_full() const { return game_loop.is_full(); }

bool Game::is_empty() const { return game_loop.is_empty(); }


std::pair<Queue<GameCommand>&, Queue<std::variant<WorldSnapshotDTO, RaceDTO>>&> Game::add_player(
        const uint32_t client_id, const std::string& username) {

    Queue<std::variant<WorldSnapshotDTO, RaceDTO>>& snapshot_queue = game_monitor.register_client(client_id);

    game_loop.add_player(client_id, username);

    if (game_loop.is_full())
        game_loop.start();

    return {command_queue, snapshot_queue};
}

void Game::remove_player(const uint32_t client_id) {

    game_monitor.remove_client(client_id);

    std::cerr << "[" << GAME << "] "
            << "Removing player with ID: " << client_id << std::endl;
}

const std::string Game::get_name() const {
    return name;
}

uint8_t Game::get_max_players() const {
    return max_players;
}

uint8_t Game::get_active_players() const {
    return game_loop.get_active_players();
}


bool Game::has_started() const {

    return game_loop.is_alive();
}


void Game::stop() {

    game_loop.stop();
    game_loop.join();

    command_queue.close();
    game_monitor.stop();

    std::cerr << "[" << GAME << "] "
            << "Game with ID: " << game_id << " stopped." << std::endl;
}

Game::~Game() {

    try {

        Game::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << GAME << "] " << EXCEPTION_IN_DESTRUCTOR << e.what();
    }
}
