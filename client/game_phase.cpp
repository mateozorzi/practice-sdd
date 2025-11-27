#include "game_phase.h"

#include <memory>

#include <SDL2/SDL.h>

#include "game.h"

GamePhase::GamePhase(Game& game, int starting_it): game(game), it(starting_it), FPS_CLIENT(60) {}

bool GamePhase::run() {
    bool loop = true;
    bool request_phase_change = false;
    const uint32_t frame_delay = 1000 / FPS_CLIENT;  // Delay en milisegundos para 60 FPS
    while (loop) {
        uint32_t frame_start = SDL_GetTicks();
        
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
        
        // Control de frame rate para evitar consumo excesivo de CPU
        uint32_t frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay) {
            SDL_Delay(frame_delay - frame_time);
        }
    }

    return request_phase_change;
}

WaitingForGamePhase::WaitingForGamePhase(Game& game, int starting_it):
        GamePhase(game, starting_it) {}
void WaitingForGamePhase::handle_game_events() { game.handle_waiting_events(); }
bool WaitingForGamePhase::update_game_state() { return game.update_waiting(); }
void WaitingForGamePhase::show_game() { game.show_waiting(); }


UpgradePhase::UpgradePhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void UpgradePhase::handle_game_events() { game.handle_upgrade_events(); }
bool UpgradePhase::update_game_state() { return game.update_upgrade(); }
void UpgradePhase::show_game() { game.show_upgrade(); }


CountdownPhase::CountdownPhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void CountdownPhase::handle_game_events() { game.handle_countdown_events(); }
bool CountdownPhase::update_game_state() { return game.update_countdown(); }
void CountdownPhase::show_game() { game.show_countdown(); }

RacePhase::RacePhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void RacePhase::handle_game_events() { game.handle_race_events(); }
bool RacePhase::update_game_state() { return game.update_race(); }
void RacePhase::show_game() { game.show_race(); }


LeaderboardPhase::LeaderboardPhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void LeaderboardPhase::handle_game_events() { game.handle_leaderboard_events(); }
bool LeaderboardPhase::update_game_state() { return game.update_leaderboard(); }
void LeaderboardPhase::show_game() { game.show_leaderboard(); }

GameFinishedPhase::GameFinishedPhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void GameFinishedPhase::handle_game_events() {game.handle_game_finished_events(); }
bool GameFinishedPhase::update_game_state() { return game.update_game_finished(); }
void GameFinishedPhase::show_game() {game.show_game_finished(); }

WinnerPhase::WinnerPhase(Game& game, int starting_it): GamePhase(game, starting_it) {}
void WinnerPhase::handle_game_events() {game.handle_winner_events(); }
bool WinnerPhase::update_game_state() { return game.update_winner(); }
void WinnerPhase::show_game() {game.show_winner(); }