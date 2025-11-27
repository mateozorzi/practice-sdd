#include "game_phase.h"

#include <memory>

#include "game.h"

GamePhase::GamePhase(Game& game, int starting_it): game(game), it(starting_it), FPS_CLIENT(60) {}

bool GamePhase::run() {
    bool loop = true;
    bool request_phase_change = false;
    while (loop) {
        show_game();
        handle_game_events();
        if (!game.keep_running)
            break;
        if (!update_game_state()) {
            request_phase_change = true;
            loop = false;
        }

        if (!game.keep_running)
            break;
    }

    return request_phase_change;
}

WaitingForGamePhase::WaitingForGamePhase(Game &game, int starting_it) : GamePhase(game, starting_it)
{} 
void WaitingForGamePhase::handle_game_events() { game.handle_waiting_events(); }
bool WaitingForGamePhase::update_game_state() { return game.update_waiting(); }
void WaitingForGamePhase::show_game() { game.show_waiting(); }


UpgradePhase::UpgradePhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void UpgradePhase::handle_game_events() { game.handle_upgrade_events(); }
bool UpgradePhase::update_game_state() { return game.update_upgrade(); }
void UpgradePhase::show_game() { game.show_upgrade(); }

RacePhase::RacePhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void RacePhase::handle_game_events() { game.handle_race_events(); }
bool RacePhase::update_game_state() { return game.update_race(); }
void RacePhase::show_game() { game.show_race(); }

/*
StatsPhase::StatsPhase(Game &game, int starting_it) : GamePhase(game, starting_it) {}
void StatsPhase::handle_game_events() { game.handle_stats_events(); }
bool StatsPhase::update_game_state() { return game.update_stats(); }
void StatsPhase::show_game(const int &it) { game.show_stats(); }
*/
