#include "game_logic.h"

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "../common/constants.h"
#include "../common/race_dto.h"


GameLogic::GameLogic(GameView& view, Queue<GameUpdateDTO>& updates, uint32_t id):
        view(view), updates(updates), game_state{id, {}, {}, {}}, leaderboard(LeaderboardDTO()) {}


void GameLogic::reset_events() {
    for (auto& [username, p]: game_state.players) {

        p.crashed = false;
        p.isMoving = false;
    }
    game_state.player.crashed = false;
    game_state.player.isMoving = false;
}

void GameLogic::detect_events_for_player(PlayerInfo& info, const PlayerDTO& dto,
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

void GameLogic::update_player(const PlayerDTO& player, const CarPositionDTO& car,
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

bool GameLogic::collected_by_me(const PowerUpDTO& power_up) const {
    return !power_up.active && power_up.collected_by == game_state.id;
}

// Plays the sound once per pickup: a power-up stays collected by the player until it respawns
void GameLogic::detect_own_power_up_pickups(const std::vector<PowerUpDTO>& power_ups) {
    for (const auto& power_up: power_ups) {

        if (!collected_by_me(power_up))
            continue;

        auto previous = std::find_if(
                game_state.power_ups.begin(), game_state.power_ups.end(),
                [&power_up](const PowerUpDTO& candidate) { return candidate.id == power_up.id; });
        if (previous == game_state.power_ups.end() || !collected_by_me(*previous))
            view.play_power_up_sound();
    }
}

void GameLogic::update_game_state_from_snapshot(const WorldSnapshotDTO& snapshot) {

    std::unordered_map<uint32_t, PlayerInfo> last_state = std::move(game_state.players);
    game_state.players.clear();

    for (const auto& [playerDTO, carPos]: snapshot.car_positions) {
        update_player(playerDTO, carPos, last_state);
    }

    game_state.npcs = snapshot.npc_positions;

    detect_own_power_up_pickups(snapshot.power_ups);
    game_state.power_ups = snapshot.power_ups;

    // Only the effects of the own player are shown
    game_state.effects.clear();
    std::copy_if(snapshot.effects.begin(), snapshot.effects.end(),
                 std::back_inserter(game_state.effects), [this](const PowerUpEffectDTO& effect) {
                     return effect.player_id == game_state.id;
                 });
}


bool GameLogic::update_waiting() {

    GameUpdateDTO dto;
    reset_events();

    while (updates.try_pop(dto)) {

        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY ||
            dto.gameloop_action == GameLoopAction::DISCONNECT) {

            keep_running = false;
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::START_SHOP) {
            std::vector<uint32_t> ids;
            ids.push_back(game_state.id);
            for (const auto& [id, player]: game_state.players) {
                ids.push_back(id);
            }
            view.set_sound_info(ids);
            game_state.phase = Phase::UPGRADE;
            game_state.penalty_per_upgrade = std::get<uint8_t>(dto.data);
            return false;
        }
    }
    return true;
}

bool GameLogic::update_upgrade() {

    view.open_shop();

    GameUpdateDTO dto;
    WorldSnapshotDTO last;
    reset_events();
    bool got_snapshot = false;
    while (updates.try_pop(dto)) {

        if (got_snapshot) {
            update_game_state_from_snapshot(last);
            got_snapshot = false;
        }


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY ||
            dto.gameloop_action == GameLoopAction::DISCONNECT) {

            keep_running = false;
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::START_COUNTDOWN) {

            game_state.phase = Phase::COUNTDOWN;
            if (got_snapshot)
                update_game_state_from_snapshot(last);

            view.close_shop();
            return false;
        } else if (dto.gameloop_action == GameLoopAction::SEND_RACE) {

            if (std::holds_alternative<RaceDTO>(dto.data)) {

                RaceDTO race = std::get<RaceDTO>(dto.data);
                if (current_city != race.city) {

                    current_city = race.city;
                    view.load_city(race.city, race.bridges_data, race.decorations_data);
                }
                view.open_shop();
            }
        } else if (dto.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {

            if (std::holds_alternative<WorldSnapshotDTO>(dto.data)) {
                last = std::get<WorldSnapshotDTO>(dto.data);
                got_snapshot = true;
            }
        } else if (dto.gameloop_action == GameLoopAction::SEND_UPGRADES_REMAINING) {
            if (std::holds_alternative<uint8_t>(dto.data)) {
                uint8_t upgrades_remaining = std::get<uint8_t>(dto.data);
                game_state.player.upgrades_remaining = upgrades_remaining;
            }
        } else if (dto.gameloop_action == GameLoopAction::SEND_UPGRADE_SHOP_TIME_REMAINING) {
            if (std::holds_alternative<uint8_t>(dto.data)) {
                uint8_t time_remaining = std::get<uint8_t>(dto.data);
                game_state.upgrade_shop_countdown = time_remaining;
            }
        }
    }

    return true;
}

bool GameLogic::update_countdown() {

    GameUpdateDTO dto;
    WorldSnapshotDTO last;
    bool got_snapshot = false;

    while (updates.try_pop(dto)) {

        if (got_snapshot) {
            update_game_state_from_snapshot(last);
            got_snapshot = false;
        }


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY ||
            dto.gameloop_action == GameLoopAction::DISCONNECT) {

            keep_running = false;
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

bool GameLogic::update_race() {

    GameUpdateDTO dto;
    WorldSnapshotDTO last;
    bool got_snapshot = false;

    while (updates.try_pop(dto)) {


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY ||
            dto.gameloop_action == GameLoopAction::DISCONNECT) {

            keep_running = false;
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::LEADERBOARD) {

            game_state.phase = Phase::LEADERBOARD;
            if (got_snapshot)
                update_game_state_from_snapshot(last);

            game_state.player.time = 0;
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
        } else if (dto.gameloop_action == GameLoopAction::RACE_TIME) {
            if (std::holds_alternative<uint32_t>(dto.data)) {
                uint32_t elapsed_seconds = std::get<uint32_t>(dto.data);
                game_state.player.time = static_cast<int>(elapsed_seconds);
            }
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

bool GameLogic::update_leaderboard() {

    GameUpdateDTO dto;
    while (updates.try_pop(dto)) {


        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY ||
            dto.gameloop_action == GameLoopAction::DISCONNECT) {

            keep_running = false;
            view.close_leaderboard();
            return true;
        }
        if (dto.gameloop_action == GameLoopAction::GAME_FINISHED) {

            game_state.phase = Phase::GAME_FINISHED;
            view.close_leaderboard();
            return false;
        } else if (dto.gameloop_action == GameLoopAction::SEND_LEADERBOARD) {

            if (std::holds_alternative<LeaderboardDTO>(dto.data)) {

                LeaderboardDTO leaderboard_recv = std::get<LeaderboardDTO>(dto.data);
                leaderboard = std::move(leaderboard_recv);
                view.open_leaderboard();
            }
        } else if (dto.gameloop_action == GameLoopAction::START_SHOP) {

            game_state.phase = Phase::UPGRADE;
            return false;
        }
    }
    return true;
}

bool GameLogic::update_game_finished() {

    GameUpdateDTO dto;
    while (updates.try_pop(dto)) {

        if (dto.gameloop_action == GameLoopAction::RETURN_TO_LOBBY ||
            dto.gameloop_action == GameLoopAction::DISCONNECT) {

            keep_running = false;
            view.close_leaderboard();
            return true;
        } else if (dto.gameloop_action == GameLoopAction::SEND_LEADERBOARD) {

            if (std::holds_alternative<LeaderboardDTO>(dto.data)) {

                LeaderboardDTO leaderboard_recv = std::get<LeaderboardDTO>(dto.data);
                leaderboard = std::move(leaderboard_recv);
                view.open_leaderboard();
            }
        }
    }
    return true;
}


LocalInfo& GameLogic::get_state() { return game_state; }

const LeaderboardDTO& GameLogic::get_leaderboard() const { return leaderboard; }

bool GameLogic::is_running() const { return keep_running; }

void GameLogic::stop() { keep_running = false; }
