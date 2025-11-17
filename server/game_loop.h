#ifndef GAME_LOOP_H
#define GAME_LOOP_H


#include <functional>
#include <string>
#include <unordered_map>

#include "../common/game_command_dto.h"
#include "../common/thread.h"

#include "collision_listener.h"
#include "game_monitor.h"
#include "player.h"
#include "server_config.h"
#include "../common/race_catalogue_dto.h"
#include "../common/leaderboard_dto.h"


constexpr float GAME_STEP = 1.0f / 60.0f;
constexpr int VELOCITY_ITERATIONS = 10;
constexpr int POSITION_ITERATIONS = 6;


class GameLoop final: public Thread {


    const uint8_t max_players;
    const ServerConfig& config;
    GameMonitor& game_monitor;
    Queue<GameCommand>& command_queue;
    b2World world;
    std::unordered_map<uint32_t, Player> players;
    CollisionListener collision_listener;
    std::unordered_map<ClientAction, std::function<void(const GameCommand&)>> command_handlers;
    RaceCatalogueDTO& selected_races;
    std::vector<LeaderboardDTO> leaderboards;
    LeaderboardDTO actual_race;
    LeaderboardDTO final_leaderboard;

    void process_commands();
    void handle_car_selection(const GameCommand& game_command);
    void handle_car_movement(const GameCommand& game_command);
    void handle_disconnection(const GameCommand& game_command);
    void handle_finish_race(const GameCommand& game_command);
    void handle_command(const GameCommand& game_command);
    void update_positions();
    void start_race(const RaceDTO& race);
    void send_race(const RaceDTO& race);
    void send_finish_race_timeout();

    bool all_player_in_game() const;
    bool all_player_finish_race() const;


public:
    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;

    explicit GameLoop(GameMonitor& game_monitor, Queue<GameCommand>& command_queue,
                      uint8_t max_players, const ServerConfig& config, RaceCatalogueDTO& selected);

    void run() override;
    void stop() override;

    void remove_player(uint32_t client_id);

    bool is_full() const;
    bool is_empty() const;
    uint8_t get_active_players() const;

    void add_player(uint32_t client_id, const std::string& username);

    ~GameLoop() override;
};


#endif
