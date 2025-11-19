#ifndef PLAYER_H
#define PLAYER_H


#include <cstdint>
#include <memory>
#include <string>

#include "../common/car_position_dto.h"
#include "../common/movement_dto.h"
#include "../common/race_dto.h"
#include "../common/player_race_stats_dto.h"
#include "../common/player_dto.h"
#include "../common/upgrade_dto.h"

#include "car.h"
#include "server_config.h"


class Player {

    uint32_t client_id;
    std::string username;
    std::unique_ptr<Car> car;
    uint16_t health;
    uint16_t races_won;
    bool in_game;
    bool in_race;
    bool has_finish_race;
    int id_next_checkpoint;
    uint8_t n_lap;
    CheckpointDTO next;
    uint32_t race_time;
    bool dead;

    void update_next_checkpoint(const RaceDTO& race);
    bool reached_checkpoint(uint32_t x, uint32_t y, const CheckpointDTO& checkpoint);


public:
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    explicit Player(uint32_t client_id, const std::string& username, b2World& world,
                    const ServerConfig& config);

    uint32_t get_client_id() const;
    const std::string& get_username() const;
    const car_dto get_car() const;
    void set_position_for_race(const std::vector<CheckpointDTO>& checkpoints);
    b2Body* get_car_body() const;

    void handle_car_movement(const MovementDTO& movement_dto);
    void update_car_position(const RaceDTO& race);
    CarPositionDTO get_car_position() const;
    void set_car_collision(bool colliding);

    bool is_in_game() const;
    void set_in_game();

    void start_race();
    bool is_in_race() const;
    void finish_race();
    bool has_finished_race() const;
    bool have_time_recorded() const;
    void record_time(uint32_t seconds);
    uint32_t get_recorded_time() const;

    bool is_dead() const;

    void set_upgrade(const UpgradeDTO& upgrade);

    PlayerDTO get_info() const;

    PlayerRaceStatsDTO get_race_stats() const;

    void set_car_configuration(const car_dto& car_configuration, const CarClientConfig& car);

    CarLayer get_car_layer() const;
    void set_car_layer(CarLayer layer);
};


#endif
