#include "game.h"

#include <utility>
#include <vector>

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
        p.isMoving = false;
    }
    game_state.player.crashed = false;
    game_state.player.isMoving = false;
}

void Game::detect_events_for_player(PlayerInfo& info, const PlayerDTO& dto,
                                    const CarPositionDTO& car_pos) {
    // Detectar movimiento
    if (info.x != (int)car_pos.x_position || info.y != (int)car_pos.y_position ||
        info.angle != (int)car_pos.angle) {
        info.isMoving = true;
    }

    // Detectar muerte
    if (dto.dead && !info.died) {
        info.died = true;
        return;
    }

    // Detectar choque (reduccion de vida)
    if (dto.health < info.life && dto.health > 0) {
        info.crashed = true;
    }
}

void Game::update_player(const PlayerDTO& player, const CarPositionDTO& car,
                         const std::unordered_map<uint32_t, PlayerInfo>& last_state) {
    if (player.id == game_state.id) {
        PlayerInfo& info = game_state.player;

        detect_events_for_player(info, player, car);

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

    auto last_it = last_state.find(player.id);
    if (last_it != last_state.end()) {
        info = last_it->second;
        detect_events_for_player(info, player, car);
    }

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

    std::unordered_map<uint32_t, PlayerInfo> last_state = std::move(game_state.players);
    game_state.players.clear();

    for (const auto& [playerDTO, carPos]: snapshot.get_car_positions()) {
        update_player(playerDTO, carPos, last_state);
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
            std::vector<uint32_t> ids;
            ids.push_back(game_state.id);
            for (const auto& [id, player]: game_state.players) {
                ids.push_back(id);
            }
            sdl.set_sound_info(ids);
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
                last = std::get<WorldSnapshotDTO>(dto.data);
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
                last = std::get<WorldSnapshotDTO>(dto.data);
                got_snapshot = true;
            }
        } else if (dto.gameloop_action == GameLoopAction::COUNTDOWN_TIME) {
            if (std::holds_alternative<uint8_t>(dto.data)) {
                uint8_t seconds_left = std::get<uint8_t>(dto.data);
                game_state.countdown_seconds = seconds_left;
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
                last = std::get<WorldSnapshotDTO>(dto.data);
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

void Game::show_race() { sdl.render(game_state); }

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
            sdl.play_background_music("client/assets/music/trance-loop.wav");
            break;
        case Phase::COUNTDOWN:
            phase = std::make_unique<CountdownPhase>(*this, 0);
            break;
        case Phase::IN_RACE:
            phase = std::make_unique<RacePhase>(*this, 0);
            break;
        case Phase::LEADERBOARD:
            phase = std::make_unique<LeaderboardPhase>(*this, 0);
            sdl.stop_background_music();
            break;
        case Phase::GAME_FINISHED:
            phase = std::make_unique<GameFinishedPhase>(*this, 0);
            break;
    }
}
