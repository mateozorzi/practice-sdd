#include "game.h"
#include <memory>

#include "../common/queue.h"

class Lobby;

Game::Game(Lobby& lobby) : protocol(std::move()), sdl(SDLManager()), input_handler(sdl, this->protocol),
                           receiver(this->protocol), game_state(), keep_running(true) {
    phase = std::make_unique<UpgradePhase>(*this, 0);
}

void Game::run() {
    try {
        while (keep_running) {
            phase->run();
        }
    } catch (const std::exception& e) {
        this->keep_running = false;
    }
}

void Game::reset_events() {
    for (auto& [username, p] : game_state.players) {
        p.crashed = false;
        p.died = false;
        p.isMoving = false;
    }
    game_state.player.crashed = false;
    game_state.player.isMoving = false;
    game_state.player.died = false;
}

void Game::update_player(const PlayerDTO& player, const CarPositionDTO& car) {
    if (player.id == game_state.player.id) {
        game_state.player.x = car.x_position;
        game_state.player.y = car.y_position;
        game_state.player.angle = car.angle;
        game_state.player.life = player.health;
    } else {
        auto it = game_state.players.find(player.username);
        if (it != game_state.players.end()) {
            it->second.x = car.x_position;
            it->second.y = car.y_position;
            it->second.angle = car.angle;
            it->second.life = player.health;
        }
    }
}

void Game::handle_upgrade_events() {
    this->keep_running = input_handler.handle_buy_events(game_state.player.time);
}

bool Game::update_upgrade() {
    GameUpdateDTO dto;
    bool pop = true;
    WorldSnapshotDTO last;
    bool got_snapshot;
    reset_events();
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();
    while (pop) {
        if (!updates.try_pop(dto)) {
            pop = false;
            continue;
        }

        WorldSnapshotDTO snapshot = std::get<WorldSnapshotDTO>(dto.data);
        last = std::move(snapshot);
        got_snapshot = true;

        if (game_state.phase != Phase::UPGRADE) {
            if (got_snapshot) {
                update_game_state_from_snapshot(last);
            }
            sdl.close_shop();
            return false;
        }
    }

    if (got_snapshot) {
        update_game_state_from_snapshot(last);
    }
    return true;
}

void Game::show_upgrade(const int &it) {
    sdl.clear_screen();
    sdl.render();
    sdl.render_shop(game_state.player.time);
    sdl.show_screen();
}

void Game::handle_race_events() {
    this->keep_running = input_handler.handle_movement();
}

bool Game::update_race() {
    GameUpdateDTO dto;
    WorldSnapshotDTO last;
    bool got_snapshot = false;
    bool pop = true;
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();

    while (pop) {
        if (!updates.try_pop(dto)) {
            pop = false;
            continue;
        }

        WorldSnapshotDTO snapshot = std::get<WorldSnapshotDTO>(dto.data);
        detect_events(snapshot);
        last = std::move(snapshot);
        got_snapshot = true;

        if (game_state.phase != Phase::RACE) {
            if (got_snapshot) {
                update_game_state_from_snapshot(last);
                return false;
            }
        }
    }

    if (got_snapshot) {
        update_game_state_from_snapshot(last);
    }
    return true;
}


void Game::show_race(const int &it) {
    sdl.clear_screen();
    sdl.render(game_state, it);
    sdl.show_screen();
}

void Game::change_phase(std::unique_ptr<GamePhase> new_phase) {
    phase = std::move(new_phase);
}

