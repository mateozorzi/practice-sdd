#ifndef GAME_H
#define GAME_H


#include <memory>
#include <string>

#include "client_protocol.h"
#include "game_phase.h"
#include "game_state.h"
#include "input_manager.h"
#include "receiver.h"
#include "sdl_manager.h"

class Game {
private:
    ClientProtocol& protocol;
    SDLManager sdl;
    InputManager input_handler;
    Receiver receiver;
    LocalInfo game_state;
    std::unique_ptr<GamePhase> phase;
    bool keep_running;
    std::string current_city;  // Ciudad del mapa actualmente cargado
    LeaderboardDTO leaderboard;

    friend class GamePhase;
    friend class WaitingForGamePhase;
    friend class UpgradePhase;
    friend class CountdownPhase;
    friend class RacePhase;
    friend class LeaderboardPhase;
    friend class GameFinishedPhase;
    friend class WinnerPhase;

    void reset_events();

    void detect_events(const WorldSnapshotDTO& snapshot);

    void update_game_state_from_snapshot(const WorldSnapshotDTO& snapshot);


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

    void update_player(const PlayerDTO& player, const CarPositionDTO& car);

public:
    explicit Game(ClientProtocol& protocol, uint32_t id);
    void run();
};

#endif
