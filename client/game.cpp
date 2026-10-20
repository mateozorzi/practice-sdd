#include "game.h"

#include <iostream>
#include <memory>
#include <vector>

#include "../common/client_event_dto.h"
#include "../common/constants.h"
#include "../common/leaderboard_dto.h"

#include "sender.h"


Game::Game(ClientProtocol& protocol, uint32_t id):
        protocol(protocol),
        sdl(SDLManager()),
        input_handler(sdl, this->protocol),
        receiver(this->protocol),
        logic(*this, receiver.get_car_positions_queue(), id) {
    phase = std::make_unique<WaitingForGamePhase>(*this, 0);
}


bool Game::run() {

    this->receiver.start();

    try {

        while (logic.is_running()) {

            bool phase_changed = false;
            phase_changed = phase->run();

            if (phase_changed) {
                change_phase();
            }
        }

    } catch (const std::exception& e) {

        logic.stop();
    }

    bool server_connected = receiver.is_server_connected();
    this->receiver.stop();
    this->receiver.join();

    return server_connected;
}


void Game::set_sound_info(const std::vector<uint32_t>& ids) { sdl.set_sound_info(ids); }

void Game::load_city(const std::string& city_name, const std::string& bridges_data,
                     const std::string& decorations_data) {
    sdl.load_city(city_name, bridges_data, decorations_data);
}

void Game::open_shop() {
    if (!sdl.is_shop_open())
        sdl.toggle_shop();
}

void Game::close_shop() { sdl.close_shop(); }

void Game::open_leaderboard() { sdl.open_leaderboard(); }

void Game::close_leaderboard() { sdl.close_leaderboard(); }

bool Game::is_running() const { return logic.is_running(); }


void Game::handle_waiting_events() { input_handler.handle_waiting(); }

bool Game::update_waiting() { return logic.update_waiting(); }

void Game::show_waiting() {

    sdl.clear_screen();
    sdl.render_waiting();
    sdl.show_screen();
}

void Game::handle_upgrade_events() {
    input_handler.handle_buy_events(logic.get_state().player.time);
}

bool Game::update_upgrade() { return logic.update_upgrade(); }

void Game::show_upgrade() {

    const LocalInfo& game_state = logic.get_state();

    sdl.clear_screen();
    sdl.render_shop(game_state.player.time, static_cast<int>(game_state.penalty_per_upgrade),
                    static_cast<int>(game_state.player.upgrades_remaining),
                    static_cast<int>(game_state.upgrade_shop_countdown));
    sdl.show_screen();
}

void Game::handle_countdown_events() { input_handler.handle_countdown(); }

bool Game::update_countdown() { return logic.update_countdown(); }

void Game::show_countdown() { sdl.render_countdown(logic.get_state()); }


void Game::handle_race_events() { input_handler.handle_movement(); }

bool Game::update_race() { return logic.update_race(); }

void Game::show_race() { sdl.render(logic.get_state()); }

void Game::handle_leaderboard_events() { input_handler.handle_leaderboard(); }

bool Game::update_leaderboard() { return logic.update_leaderboard(); }

void Game::show_leaderboard() {

    sdl.clear_screen();
    sdl.render_leaderboard(logic.get_leaderboard(), "Leaderboard");
    sdl.show_screen();
}

void Game::handle_game_finished_events() { input_handler.handle_game_finished(); }

bool Game::update_game_finished() { return logic.update_game_finished(); }

void Game::show_game_finished() {

    sdl.clear_screen();
    sdl.render_leaderboard(logic.get_leaderboard(), "Final Leaderboard");
    sdl.show_screen();
}


void Game::change_phase() {

    switch (logic.get_state().phase) {

        case Phase::WAITING_FOR_CAR_SELECTION:
            phase = std::make_unique<WaitingForGamePhase>(*this, 0);
            break;
        case Phase::UPGRADE:
            phase = std::make_unique<UpgradePhase>(*this, 0);
            sdl.play_background_music();
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
            sdl.play_finished_race_sound();
            break;
        case Phase::GAME_FINISHED:
            phase = std::make_unique<GameFinishedPhase>(*this, 0);
            break;
    }
}
