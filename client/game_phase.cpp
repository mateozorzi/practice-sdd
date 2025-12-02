#include "game_phase.h"

#include <memory>

#include <SDL2/SDL.h>

#include "game.h"

GamePhase::GamePhase(Game& game, int starting_it, int fps): game(game), it(starting_it), FPS_CLIENT(fps) {}

bool GamePhase::run() {
    bool loop = true;
    bool request_phase_change = false;
    const Uint32 frame_delay = 1000 / FPS_CLIENT;
    while (loop) {
        Uint32 frame_start = SDL_GetTicks();

        show_game();
        handle_game_events();
        if (!game.keep_running)
            break;
        if (!update_game_state()) {
            request_phase_change = true;
            loop = false;
        }

        if (!game.keep_running) {
            break;
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay) {
            SDL_Delay(frame_delay - frame_time);
        }
    }

    return request_phase_change;
}

WaitingForGamePhase::WaitingForGamePhase(Game& game, int starting_it):
        GamePhase(game, starting_it, 30) {}
void WaitingForGamePhase::handle_game_events() { game.handle_waiting_events(); }
bool WaitingForGamePhase::update_game_state() { return game.update_waiting(); }
void WaitingForGamePhase::show_game() { game.show_waiting(); }


UpgradePhase::UpgradePhase(Game& game, int starting_it): GamePhase(game, starting_it, 30) {}
void UpgradePhase::handle_game_events() { game.handle_upgrade_events(); }
bool UpgradePhase::update_game_state() { return game.update_upgrade(); }
void UpgradePhase::show_game() { game.show_upgrade(); }


CountdownPhase::CountdownPhase(Game& game, int starting_it): GamePhase(game, starting_it, 30) {}
void CountdownPhase::handle_game_events() { game.handle_countdown_events(); }
bool CountdownPhase::update_game_state() { return game.update_countdown(); }
void CountdownPhase::show_game() { game.show_countdown(); }

RacePhase::RacePhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void RacePhase::handle_game_events() { game.handle_race_events(); }
bool RacePhase::update_game_state() { return game.update_race(); }
void RacePhase::show_game() { game.show_race(); }


LeaderboardPhase::LeaderboardPhase(Game& game, int starting_it): GamePhase(game, starting_it, 30) {}
void LeaderboardPhase::handle_game_events() { game.handle_leaderboard_events(); }
bool LeaderboardPhase::update_game_state() { return game.update_leaderboard(); }
void LeaderboardPhase::show_game() { game.show_leaderboard(); }

GameFinishedPhase::GameFinishedPhase(Game& game, int starting_it): GamePhase(game, starting_it, 30) {}
void GameFinishedPhase::handle_game_events() {game.handle_game_finished_events(); }
bool GameFinishedPhase::update_game_state() { return game.update_game_finished(); }
void GameFinishedPhase::show_game() { game.show_game_finished(); }
