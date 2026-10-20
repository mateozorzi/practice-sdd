#ifndef GAME_H
#define GAME_H


#include <memory>
#include <string>
#include <vector>

#include "client_protocol.h"
#include "game_logic.h"
#include "game_phase.h"
#include "game_state.h"
#include "input_manager.h"
#include "receiver.h"
#include "sdl_manager.h"


// Runs the game phases: reads the input, lets GameLogic apply the server updates and draws
// the result with SDL.
class Game: private GameView {

    ClientProtocol& protocol;
    SDLManager sdl;
    InputManager input_handler;
    Receiver receiver;
    GameLogic logic;
    std::unique_ptr<GamePhase> phase;

    friend class GamePhase;
    friend class WaitingForGamePhase;
    friend class UpgradePhase;
    friend class CountdownPhase;
    friend class RacePhase;
    friend class LeaderboardPhase;
    friend class GameFinishedPhase;
    friend class WinnerPhase;

    // GameView
    void set_sound_info(const std::vector<uint32_t>& ids) override;
    void load_city(const std::string& city_name, const std::string& bridges_data,
                   const std::string& decorations_data) override;
    void open_shop() override;
    void close_shop() override;
    void open_leaderboard() override;
    void close_leaderboard() override;

    bool is_running() const;

    void handle_waiting_events();
    bool update_waiting();
    void show_waiting();

    void handle_upgrade_events();
    bool update_upgrade();
    void show_upgrade();

    void handle_countdown_events();
    bool update_countdown();
    void show_countdown();

    void handle_race_events();
    bool update_race();
    void show_race();

    void handle_leaderboard_events();
    bool update_leaderboard();
    void show_leaderboard();

    void handle_game_finished_events();
    bool update_game_finished();
    void show_game_finished();

    void change_phase();


public:
    explicit Game(ClientProtocol& protocol, uint32_t id);
    bool run();
};


#endif
