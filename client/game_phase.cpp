#include "game_phase.h"
#include <memory>

#include "game.h"

GamePhase::GamePhase(Game &game, int starting_it) : game(game), it(starting_it), FPS_CLIENT(60) {}

void GamePhase::run() {
    bool loop = true;

    while (loop) {
        show_game(it);
        handle_game_events();
        if (!game.keep_running)
            break;
        if (!update_game_state()) {
            loop = false;
        }

        if (!game.keep_running)
            break;
    }
}

void GamePhase::change_phase(int last_it) {
    if (game.game_state.phase == Phase::WAITING_PLAYERS) {
        game.change_phase(std::make_unique<WaitingForGamePhase>{game, 0});
    } else if (game.game_state.phase == Phase::UPGRADE) {
        game.change_phase(std::make_unique<UpgradePhase>{game, 0});
    } else if (game.game_state.phase == Phase::RACE) {
        game.change_phase(std::make_unique<RacePhase>{game, last_it});
    } else if (game.game_state.phase == Phase::STATS) {
        game.change_phase(std::make_unique<StatsPhase>{game, last_it});
    }
}

/*
WaitingForGamePhase::WaitingForGamePhase(Game &game, int starting_it) : GamePhase(game, starting_it) {}
void WaitingForGamePhase::handle_game_events() { game.handle_waiting_events(); }
bool WaitingForGamePhase::update_game_state() { return game.update_waiting(); }
void WaitingForGamePhase::show_game(const int &it) { game.show_waiting(it); }
*/

UpgradePhase::UpgradePhase(Game &game, int starting_it) : GamePhase(game, starting_it) {}
void UpgradePhase::handle_game_events() { game.handle_upgrade_events(); }
bool UpgradePhase::update_game_state() { return game.update_upgrade(); }
void UpgradePhase::show_game(const int &it) { game.show_upgrade(it); }

RacePhase::RacePhase(Game& game, int starting_it) : GamePhase(game, starting_it) {}
void RacePhase::handle_game_events() { game.handle_race_events(); }
bool RacePhase::update_game_state() { return game.update_race(); }
void RacePhase::show_game(const int &it) { game.show_race(it); }

/*
StatsPhase::StatsPhase(Game &game, int starting_it) : GamePhase(game, starting_it) {}
void StatsPhase::handle_game_events() { game.handle_stats_events(); }
bool StatsPhase::update_game_state() { return game.update_stats(); }
void StatsPhase::show_game(const int &it) { game.show_stats(); }
*/




