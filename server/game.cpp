#include "game.h"

#include "../common/constants.h"
#include "../common/destructor_log.h"
#include "../common/race_catalogue_dto.h"


Game::Game(const uint8_t max_players, const ServerConfig& config, std::string name,
           const RaceCatalogueDTO& selected_races, PowerUpTypeGenerator type_generator):
        max_players(max_players),
        config(config),
        name(std::move(name)),
        command_queue(static_cast<size_t>(config.server.command_queue_capacity)),
        game_monitor(config),
        game_loop(game_monitor, command_queue, max_players, config, selected_races,
                  std::move(type_generator)) {}


bool Game::is_full() const { return game_loop.is_full(); }

bool Game::is_empty() const { return game_loop.is_empty(); }


std::pair<Queue<GameCommand>&, Queue<GameUpdateDTO>&> Game::add_player(
        const uint32_t player_id, const std::string& username) {

    Queue<GameUpdateDTO>& snapshot_queue = game_monitor.register_player(player_id);

    game_loop.add_player(player_id, username);

    if (!game_loop.is_alive())
        game_loop.start();

    return {command_queue, snapshot_queue};
}

void Game::remove_player(const uint32_t player_id) {

    game_loop.remove_player(player_id);
    game_monitor.remove_player(player_id);
}

const std::string& Game::get_name() const { return name; }

uint8_t Game::get_max_players() const { return max_players; }

uint8_t Game::get_active_players() const { return game_loop.get_active_players(); }

uint32_t Game::get_next_player_id() const { return game_loop.get_next_player_id(); }


bool Game::has_started() const { return game_loop.has_started(); }

bool Game::has_player(const uint32_t player_id) const { return game_loop.has_player(player_id); }


void Game::stop() {

    game_loop.stop();
    game_loop.join();

    game_monitor.stop();

    command_queue.close();
}

Game::~Game() {
    log_stopping(GAME);
    try {
        Game::stop();
    } catch (const std::exception& e) {
        log_destructor_error(GAME, e.what());
    } catch (...) {
        log_destructor_error(GAME, "<unknown>");
    }
}
