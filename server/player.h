#ifndef PLAYER_H
#define PLAYER_H


#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../common/car_position_dto.h"
#include "../common/client_event_dto.h"
#include "../common/movement_dto.h"
#include "../common/player_dto.h"
#include "../common/player_race_stats_dto.h"
#include "../common/power_up_type.h"
#include "../common/race_dto.h"
#include "../common/upgrade_dto.h"

#include "car.h"
#include "server_config.h"


class Player {


    uint32_t player_id;
    std::string username;
    std::unique_ptr<Car> car;
    uint16_t health = 0;
    uint16_t races_won = 0;
    bool in_game = false;
    bool in_race = false;
    bool has_finish_race = false;
    int id_next_checkpoint = 1;
    uint8_t n_lap = 1;
    CheckpointDTO next;
    uint32_t race_time = 0;
    bool dead = false;
    uint16_t base_health = 0;
    bool penalty = false;
    int n_penalty = 0;
    bool infinite_health = false;
    bool checkpoint_triggered = false;
    double nitro_remaining = 0.0;   // seconds, active while > 0
    double shield_remaining = 0.0;  // seconds, active while > 0

    bool update_next_checkpoint(const RaceDTO& race);
    void clear_effects();


public:
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    explicit Player(uint32_t player_id, const std::string& username);

    void create_car(const std::string& car_model, const ServerConfig& car_config,
                    b2World& box2d_world);

    uint32_t get_id() const;
    const std::string& get_username() const;
    const CarDTO get_car() const;
    CarPositionDTO get_car_position(const RaceDTO& race) const;
    b2Body* get_car_body() const;
    CarLayer get_car_layer() const;
    uint32_t get_recorded_time() const;
    PlayerDTO get_info() const;
    PlayerRaceStatsDTO get_race_stats() const;
    uint16_t get_health() const;
    float get_max_speed() const;
    float get_acceleration() const;

    void handle_car_movement(const MovementDTO& movement_dto);
    void update_car_position(const RaceDTO& race, uint32_t elapsed_seconds,
                             const GameConfig& game_config);
    void set_car_collision(bool colliding);
    void set_position_for_race(const std::vector<CheckpointDTO>& checkpoints);
    void set_car_layer(CarLayer layer);
    void set_in_game();
    void set_upgrade(const BuyUpgradeDTO& upgrade, const UpgradeConfig& game_config);
    void activate_infinite_health();

    bool is_in_game() const;
    void start_race();
    bool is_in_race() const;
    bool is_next_checkpoint(uint32_t checkpoint_id) const;
    void trigger_checkpoint_collision();
    void finish_race();
    bool has_finished_race() const;
    bool have_time_recorded() const;
    void record_time(uint32_t seconds, const GameConfig& game_config);

    uint8_t get_upgrades_remaining() const;

    bool is_dead() const;

    void apply_collision_damage(uint16_t damage);

    void apply_power_up(PowerUpType type, const PowerUpConfig& power_up_config);
    void advance_effects(double elapsed_seconds);
    bool has_effect(PowerUpType type) const;
    double get_effect_remaining(PowerUpType type) const;
};


#endif
