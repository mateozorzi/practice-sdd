#ifndef PLAYER_H
#define PLAYER_H


#include <cstdint>
#include <memory>
#include <string>

#include "../common/car_position_dto.h"
#include "../common/movement_dto.h"

#include "car.h"
#include "server_config.h"


class Player {

    uint32_t client_id;
    std::string username;
    std::unique_ptr<Car> car;
    uint16_t health;
    uint16_t races_won;
    bool in_game;


public:
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    explicit Player(uint32_t client_id, const std::string& username, b2World& world,
                    const ServerConfig& config);

    uint32_t get_client_id() const;
    const std::string& get_username() const;
    const car_dto get_car() const;
    void set_position_for_race(const CheckpointDTO& start_position);
    b2Body* get_car_body() const;

    void handle_car_movement(const MovementDTO& movement_dto);
    void update_car_position();
    CarPositionDTO get_car_position() const;
    void set_car_collision(bool colliding);

    bool is_in_game() const;
    void set_in_game();

    void set_car_configuration(const car_dto& car_configuration);
};


#endif
