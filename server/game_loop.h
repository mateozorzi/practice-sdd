#ifndef GAME_LOOP_H
#define GAME_LOOP_H


#include <functional>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>


#include "../common/game_command_dto.h"
#include "../common/leaderboard_dto.h"
#include "../common/race_catalogue_dto.h"
#include "../common/thread.h"


#include "city_loader.h"
#include "collision_listener.h"
#include "game_monitor.h"
#include "player.h"
#include "server_config.h"


constexpr std::string_view GAME_LOOP = "GAME_LOOP";
constexpr float GAME_STEP = 1.0f / 60.0f;
constexpr int VELOCITY_ITERATIONS = 10;
constexpr int POSITION_ITERATIONS = 6;


enum class GamePhase { WAITING_CAR_SELECTION, UPGRADE_SHOP, COUNTDOWN, IN_RACE, LEADERBOARD, GAME_FINISHED };


class GameLoop final: public Thread {

    std::unordered_map<ClientAction, std::function<void(const GameCommand&)>> command_handlers;

    const ServerConfig& config;
    const uint8_t max_players;
    GamePhase current_phase = GamePhase::WAITING_CAR_SELECTION;
    const std::chrono::milliseconds game_iteration_duration;

    std::optional<std::chrono::steady_clock::time_point> phase_start = std::nullopt;
    std::optional<std::chrono::steady_clock::time_point> race_start_time = std::nullopt;
    std::chrono::seconds upgrade_shop_seconds;
    std::chrono::seconds countdown_seconds;
    std::chrono::seconds race_timeout_seconds;
    std::chrono::seconds leaderboard_seconds;
    std::chrono::seconds game_finish_seconds;
    uint8_t current_race_index = 0;

    GameMonitor& game_monitor;
    Queue<GameCommand>& command_queue;
    b2World box2d_world;
    CityLoader city_loader;
    std::unordered_map<uint32_t, Player> players;
    CollisionListener collision_listener;
    RaceCatalogueDTO& selected_races;
    std::vector<LeaderboardDTO> leaderboards;
    LeaderboardDTO final_leaderboard;
    bool race_active;

    void fixed_sleep(const std::chrono::steady_clock::time_point& iteration_start_time);
    void handle_waiting_car_selection_phase();
    void handle_upgrade_shop_phase();
    void handle_countdown_phase();
    void handle_in_race_phase();
    void handle_leaderboard_phase();
    void handle_game_finished_phase();

    void process_commands_in_waiting_car_selection_phase();
    void process_commands_in_upgrade_shop_phase();
    void process_commands_in_countdown_phase();
    void process_commands_in_race_phase();
    void process_commands_in_leaderboard_phase();
    void process_commands_in_game_finished_phase();

    void process_commands();
    void handle_car_selection(const GameCommand& game_command);
    void handle_car_movement(const GameCommand& game_command);
    void handle_disconnection(const GameCommand& game_command);
    void handle_upgrade_car(const GameCommand& game_command);
    void handle_command(const GameCommand& game_command);
    void update_positions(const RaceDTO& race);
    void set_race(const RaceDTO& race);
    void finish_race(uint32_t timeout_time);
    LeaderboardDTO create_final_leaderboard();
    LeaderboardDTO create_leaderboard();
    void send_race(const RaceDTO& race);
    CarConfig get_car_config(const ServerConfig& server_config, const CarDTO& car_dto);

    bool all_player_in_game() const;
    bool player_in_race() const;

    void map_configuration(const std::string& city_name);
    void shop_phase();


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

    void check_race_times(uint32_t seconds);
    void check_alive_players();

    void add_player(uint32_t client_id, const std::string& username);

    ~GameLoop() override;
};


#endif
