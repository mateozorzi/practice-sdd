#include "game_phase.h"

#include <memory>

#include <SDL2/SDL.h>

#include "game.h"

GamePhase::GamePhase(Game& game, int starting_it, int fps):
        game(game), it(starting_it), FPS_CLIENT(fps) {}

bool GamePhase::run() {
    bool loop = true;
    bool request_phase_change = false;
    const Uint32 rate = 1000 / FPS_CLIENT;
    Uint32 t1 = SDL_GetTicks();

    while (loop) {
        handle_game_events();

        if (!game.keep_running)
            break;

        if (!update_game_state()) {
            request_phase_change = true;
            loop = false;
        }

        if (!game.keep_running)
            break;

        show_game();

        Uint32 t2 = SDL_GetTicks();
        Sint32 rest = static_cast<Sint32>(rate) - static_cast<Sint32>(t2 - t1);

        if (rest < 0) {
            Uint32 behind = static_cast<Uint32>(-rest);
            rest = static_cast<Sint32>(rate - (behind % rate));
            Uint32 lost = behind + static_cast<Uint32>(rest);
            t1 += lost;
            it += static_cast<int>(lost / rate);
        }

        SDL_Delay(static_cast<Uint32>(rest));
        t1 += rate;
        it++;
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

RacePhase::RacePhase(Game& game, int starting_it): GamePhase(game, starting_it, 60) {}
void RacePhase::handle_game_events() { game.handle_race_events(); }
bool RacePhase::update_game_state() { return game.update_race(); }
void RacePhase::show_game() { game.show_race(); }


LeaderboardPhase::LeaderboardPhase(Game& game, int starting_it): GamePhase(game, starting_it, 30) {}
void LeaderboardPhase::handle_game_events() { game.handle_leaderboard_events(); }
bool LeaderboardPhase::update_game_state() { return game.update_leaderboard(); }
void LeaderboardPhase::show_game() { game.show_leaderboard(); }

GameFinishedPhase::GameFinishedPhase(Game& game, int starting_it):
        GamePhase(game, starting_it, 30) {}
void GameFinishedPhase::handle_game_events() { game.handle_game_finished_events(); }
bool GameFinishedPhase::update_game_state() { return game.update_game_finished(); }
void GameFinishedPhase::show_game() { game.show_game_finished(); }
