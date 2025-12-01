#include "game.h"

#include <utility>

#include "../common/client_event_dto.h"
#include "../common/constants.h"
#include "../common/leaderboard_dto.h"
#include "../common/queue.h"
#include "../common/race_dto.h"

#include "sender.h"


Game::Game(ClientProtocol& protocol, uint32_t id):
        protocol(protocol),
        sdl(SDLManager()),
        input_handler(sdl, this->protocol),
        receiver(this->protocol),
        game_state{id, {}, {}, {}},
        leaderboard(LeaderboardDTO()) {
    phase = std::make_unique<WaitingForGamePhase>(*this, 0);
}


void Game::run() {

    this->receiver.start();

    try {

        while (keep_running) {

            bool phase_changed = false;
            phase_changed = phase->run();

            if (phase_changed) {
                change_phase();
            }
        }

    } catch (const std::exception& e) {

        this->keep_running = false;
    }

    this->receiver.stop();
    this->receiver.join();
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

        if (it == game_state.players.end())
            continue;

        PlayerInfo& prev = it->second;
        bool moved = false;

        if (prev.x != (int)carPos.x_position || prev.y != (int)carPos.y_position ||
            prev.angle != (int)carPos.angle)
            moved = true;

        prev.isMoving = moved;
    }
}

void Game::update_player(const PlayerDTO& player, const CarPositionDTO& car) {

    if (player.id == game_state.id) {

        PlayerInfo& info = game_state.player;
        info.id = player.id;
        info.x = car.x_position;
        info.y = car.y_position;
        info.angle = car.angle;
        info.life = player.health;
        info.max_life = player.max_health;
        info.died = player.dead;
        info.layer = car.layer;
        info.checkpoint = car.next;
        info.model = player.car.model;
        info.finish_race = player.finish_race;
        info.current_lap = car.current_lap;
        info.race_lap = car.race_lap;
        info.username = player.username;
        return;
    }

    PlayerInfo& info = game_state.players[player.id];

    info.id = player.id;
    info.model = player.car.model;
    info.life = player.health;
    info.max_life = player.max_health;
    info.died = player.dead;
    info.x = car.x_position;
    info.y = car.y_position;
    info.angle = car.angle;
    info.layer = car.layer;
    info.checkpoint = car.next;
    info.finish_race = player.finish_race;
    info.current_lap = car.current_lap;
    info.race_lap = car.race_lap;
    info.username = player.username;
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

    game_state.npcs = snapshot.get_npc_positions();
}

void Game::handle_waiting_events() { input_handler.handle_waiting(); }

bool Game::update_waiting() {

    GameUpdateDTO dto;
    reset_events();

    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();

    while (updates.try_pop(dto)) {

        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

            this->keep_running = false;
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::START_SHOP) {

            game_state.phase = Phase::UPGRADE;
            return false;
        }
    }
    return true;
}

void Game::show_waiting() {

    sdl.clear_screen();
    sdl.render_waiting();
    sdl.show_screen();
}

void Game::handle_upgrade_events() { input_handler.handle_buy_events(game_state.player.time); }

bool Game::update_upgrade() {

    if (!sdl.is_shop_open())
        sdl.toggle_shop();

    GameUpdateDTO dto;
    WorldSnapshotDTO last;
    reset_events();
    bool got_snapshot = false;
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();
    while (updates.try_pop(dto)) {

        if (got_snapshot) {
            update_game_state_from_snapshot(last);
            got_snapshot = false;
        }


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

            this->keep_running = false;
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::START_COUNTDOWN) {

            game_state.phase = Phase::COUNTDOWN;
            if (got_snapshot)
                update_game_state_from_snapshot(last);

            sdl.close_shop();
            return false;
        } else if (dto.gameloop_action == GameLoopAction::SEND_RACE) {

            if (std::holds_alternative<RaceDTO>(dto.data)) {

                RaceDTO race = std::get<RaceDTO>(dto.data);
                if (current_city != race.city) {

                    current_city = race.city;
                    sdl.load_city(race.city);
                }
                if (!sdl.is_shop_open())
                    sdl.toggle_shop();
            }
        } else if (dto.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {

            if (std::holds_alternative<WorldSnapshotDTO>(dto.data)) {

                WorldSnapshotDTO snapshot = std::get<WorldSnapshotDTO>(dto.data);
                detect_events(snapshot);
                last = std::move(snapshot);
                got_snapshot = true;
            }
        }
    }

    return true;
}

void Game::show_upgrade() {

    sdl.clear_screen();
    sdl.render_shop(game_state.player.time);
    sdl.show_screen();
}

void Game::handle_countdown_events() { input_handler.handle_countdown(); }

bool Game::update_countdown() {

    GameUpdateDTO dto;
    WorldSnapshotDTO last;
    bool got_snapshot = false;
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();

    while (updates.try_pop(dto)) {

        if (got_snapshot) {
            update_game_state_from_snapshot(last);
            got_snapshot = false;
        }


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

            this->keep_running = false;
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::START_RACE) {
            game_state.phase = Phase::IN_RACE;
            return false;
        }

        if (dto.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {
            if (std::holds_alternative<WorldSnapshotDTO>(dto.data)) {
                WorldSnapshotDTO snapshot = std::get<WorldSnapshotDTO>(dto.data);
                detect_events(snapshot);
                last = std::move(snapshot);
                got_snapshot = true;
            }
        }
    }
    return true;
}

void Game::show_countdown() {

    sdl.clear_screen();
    sdl.render_countdown(game_state);
    sdl.show_screen();
}


void Game::handle_race_events() { input_handler.handle_movement(); }


bool Game::update_race() {

    GameUpdateDTO dto;
    WorldSnapshotDTO last;
    bool got_snapshot = false;
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();

    while (updates.try_pop(dto)) {


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

            this->keep_running = false;
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::LEADERBOARD) {

            game_state.phase = Phase::LEADERBOARD;
            if (got_snapshot)
                update_game_state_from_snapshot(last);

            return false;
        } else if (dto.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {

            if (std::holds_alternative<WorldSnapshotDTO>(dto.data)) {

                WorldSnapshotDTO snapshot = std::get<WorldSnapshotDTO>(dto.data);
                detect_events(snapshot);
                last = std::move(snapshot);
                got_snapshot = true;
            }
        } else if (dto.gameloop_action == GameLoopAction::GAME_FINISHED) {

            game_state.phase = Phase::GAME_FINISHED;
            if (got_snapshot)
                update_game_state_from_snapshot(last);

            return false;
        }

        if (game_state.phase != Phase::IN_RACE) {

            if (got_snapshot)
                update_game_state_from_snapshot(last);

            return false;
        }
    }

    if (got_snapshot)
        update_game_state_from_snapshot(last);

    return true;
}

void Game::show_race() {

    sdl.clear_screen();
    sdl.render(game_state);
    sdl.show_screen();
}

void Game::handle_leaderboard_events() { input_handler.handle_leaderboard(); }

bool Game::update_leaderboard() {

    GameUpdateDTO dto;
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();
    while (updates.try_pop(dto)) {


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

            this->keep_running = false;
            sdl.close_leaderboard();
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::GAME_FINISHED) {

            game_state.phase = Phase::GAME_FINISHED;
            sdl.close_leaderboard();
            return false;
        } else if (dto.gameloop_action == GameLoopAction::SEND_LEADERBOARD) {

            if (std::holds_alternative<LeaderboardDTO>(dto.data)) {

                LeaderboardDTO leaderboard_recv = std::get<LeaderboardDTO>(dto.data);
                leaderboard = std::move(leaderboard_recv);
                sdl.open_leaderboard();
            }
        } else if (dto.gameloop_action == GameLoopAction::START_SHOP) {

            game_state.phase = Phase::UPGRADE;
            return false;
        }
    }
    return true;
}

void Game::show_leaderboard() {

    sdl.clear_screen();
    sdl.render_leaderboard(leaderboard, "Leaderboard");
    sdl.show_screen();
}

void Game::handle_game_finished_events() { input_handler.handle_game_finished(); }

bool Game::update_game_finished() {

    GameUpdateDTO dto;
    Queue<GameUpdateDTO>& updates = receiver.get_car_positions_queue();
    while (updates.try_pop(dto)) {

        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {

            this->keep_running = false;
            sdl.close_leaderboard();
            return true;
        } else if (dto.gameloop_action == GameLoopAction::SEND_LEADERBOARD) {

            if (std::holds_alternative<LeaderboardDTO>(dto.data)) {

                LeaderboardDTO leaderboard_recv = std::get<LeaderboardDTO>(dto.data);
                leaderboard = std::move(leaderboard_recv);
                sdl.open_leaderboard();
            }
        }
    }
    return true;
}

void Game::show_game_finished() {

    sdl.clear_screen();
    sdl.render_leaderboard(leaderboard, "Final Leaderboard");
    sdl.show_screen();
}


void Game::change_phase() {

    switch (this->game_state.phase) {

        case Phase::WAITING_FOR_CAR_SELECTION:
            phase = std::make_unique<WaitingForGamePhase>(*this, 0);
            break;
        case Phase::UPGRADE:
            phase = std::make_unique<UpgradePhase>(*this, 0);
            break;
        case Phase::COUNTDOWN:
            phase = std::make_unique<CountdownPhase>(*this, 0);
            break;
        case Phase::IN_RACE:
            phase = std::make_unique<RacePhase>(*this, 0);
            break;
        case Phase::LEADERBOARD:
            phase = std::make_unique<LeaderboardPhase>(*this, 0);
            break;
        case Phase::GAME_FINISHED:
            phase = std::make_unique<GameFinishedPhase>(*this, 0);
            break;
    }
}
