#include "game.h"

#include <utility>

#include "../common/constants.h"
#include "../common/queue.h"
#include "../common/race_dto.h"

Game::Game(ClientProtocol& protocol, uint32_t id):
        protocol(protocol),
        sdl(SDLManager()),
        input_handler(sdl, this->protocol),
        receiver(this->protocol),
        game_state{id, {}, {}, {}},
        keep_running(true),
        current_city("") {
    phase = std::make_unique<RacePhase>(*this, 0);
}

void Game::run() {
    this->receiver.start();
    try {
        while (keep_running) {
            phase->run();
        }
    } catch (const std::exception& e) {
        this->keep_running = false;
    }
    this->receiver.stop();
    this->receiver.join();
    protocol.send_client_action(ClientAction::DISCONNECT);
}

void Game::reset_events() {
    for (auto& [username, p]: game_state.players) {
        p.crashed = false;
        p.died = false;
        p.isMoving = false;
    }
    game_state.player.crashed = false;
    game_state.player.isMoving = false;
    game_state.player.died = false;
}

void Game::detect_events(const WorldSnapshotDTO& snapshot) {
    for (const auto& [playerDTO, carPos]: snapshot.get_car_positions()) {
        const uint32_t& id = playerDTO.id;
        auto it = game_state.players.find(id);

        if (it == game_state.players.end()) {
            continue;
        }

        PlayerInfo& prev = it->second;
        bool moved = false;

        if (prev.x != (int)carPos.x_position || prev.y != (int)carPos.y_position ||
            prev.angle != (int)carPos.angle) {
            moved = true;
        }

        prev.isMoving = moved;
    }
}

void Game::update_player(const PlayerDTO& player, const CarPositionDTO& car) {
    if (player.id == game_state.player.id) {
        PlayerInfo& info = game_state.player;
        info.id = player.id;
        info.x = car.x_position;
        info.y = car.y_position;
        info.angle = car.angle;
        info.life = player.health;
        info.max_life = player.car.health;
        info.died = player.dead;
        info.layer = car.layer;
        info.checkpoint = car.next;
        info.model = player.car.model;
        return;
    }

    PlayerInfo& info = game_state.players[player.id];

    info.id = player.id;
    info.model = player.car.model;
    info.life = player.health;
    info.max_life = player.car.health;
    info.died = player.dead;
    info.x = car.x_position;
    info.y = car.y_position;
    info.angle = car.angle;
    info.layer = car.layer;
    info.checkpoint = car.next;
}

void Game::update_game_state_from_snapshot(const WorldSnapshotDTO& snapshot) {
    game_state.players.clear();

    for (const auto& [playerDTO, carPos]: snapshot.get_car_positions()) {
        if (playerDTO.username == game_state.player.model || playerDTO.id == game_state.player.id) {
            update_player(playerDTO, carPos);
            continue;
        }
        update_player(playerDTO, carPos);
    }
}

void Game::handle_upgrade_events() {
    this->keep_running = input_handler.handle_buy_events(game_state.player.time);
}

bool Game::update_upgrade() {
    GameUpdateDTO dto;
    bool pop = true;
    WorldSnapshotDTO last;
    bool got_snapshot = false;
    reset_events();
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();
    while (pop) {
        if (!updates.try_pop(dto)) {
            pop = false;
            continue;
        }

        if (dto.gameloop_action == GameLoopAction::CLOSE_SHOP) {
            game_state.phase = Phase::RACE;
            if (got_snapshot) {
                update_game_state_from_snapshot(last);
            }
            sdl.close_shop();
            return false;
        } else if (dto.gameloop_action == GameLoopAction::SEND_RACE) {
            // Detectar cuando se recibe una nueva carrera - Deberia hacerse durante etapa
            // especifica (waiting o endgame) - Usar variable de sdl manager
            if (std::holds_alternative<RaceDTO>(dto.data)) {
                RaceDTO race = std::get<RaceDTO>(dto.data);
                if (current_city != race.city) {
                    current_city = race.city;
                    sdl.load_city(race.city);
                }
            }
        } else if (dto.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {
            if (std::holds_alternative<WorldSnapshotDTO>(dto.data)) {
                WorldSnapshotDTO snapshot = std::get<WorldSnapshotDTO>(dto.data);
                last = std::move(snapshot);
                got_snapshot = true;
            }
        }

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

void Game::show_upgrade() {
    sdl.clear_screen();
    sdl.render(game_state);
    sdl.render_shop(game_state.player.time);
    sdl.show_screen();
}

void Game::handle_race_events() { this->keep_running = input_handler.handle_movement(); }

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

        if (dto.gameloop_action == GameLoopAction::START_SHOP) {
            game_state.phase = Phase::UPGRADE;
            if (got_snapshot) {
                update_game_state_from_snapshot(last);
            }
            return false;
        } else if (dto.gameloop_action == GameLoopAction::SEND_RACE) {
            if (std::holds_alternative<RaceDTO>(dto.data)) {
                RaceDTO race = std::get<RaceDTO>(dto.data);
                if (current_city != race.city) {
                    current_city = race.city;
                    sdl.load_city(race.city);
                }
            }
        } else if (dto.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {
            if (game_state.phase == Phase::WAITING_PLAYERS) {
                game_state.phase = Phase::RACE;
            }
            if (std::holds_alternative<WorldSnapshotDTO>(dto.data)) {
                WorldSnapshotDTO snapshot = std::get<WorldSnapshotDTO>(dto.data);
                detect_events(snapshot);
                last = std::move(snapshot);
                got_snapshot = true;
            }
        } else if (dto.gameloop_action == GameLoopAction::CLOSE_SHOP) {
            game_state.phase = Phase::RACE;
        }

        if (game_state.phase != Phase::RACE) {
            if (got_snapshot) {
                update_game_state_from_snapshot(last);
            }
            return false;
        }
    }

    if (got_snapshot) {
        update_game_state_from_snapshot(last);
    }
    return true;
}

void Game::show_race() {
    sdl.clear_screen();
    sdl.render(game_state);
    sdl.show_screen();
}

void Game::change_phase(std::unique_ptr<GamePhase> new_phase) { phase = std::move(new_phase); }
