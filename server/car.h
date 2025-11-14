#ifndef CAR_H
#define CAR_H


#include <box2d/box2d.h>

#include "../common/car_dto.h"
#include "../common/car_position_dto.h"
#include "../common/movement_dto.h"
#include "server_config.h"


struct CarConfig {

    float width;
    float height;
    float density;
    float friction;
    float restitution;

    float linear_damping;
    float angular_damping;
    float min_turn_speed;

    float acceleration;
    float breaking_factor;
    float max_speed;
    float drift_factor;
    float turn_torque;

    float mass;
    int health;
};


class Car {

    uint32_t client_id;
    b2Body* body;
    CarConfig config;

    bool moving_up = false;
    bool moving_down = false;
    bool moving_left = false;
    bool moving_right = false;
    bool is_colliding = false;

    void detect_linear_movement();
    void detect_rotation();
    void detect_drifting();

public:
    Car(const Car&) = delete;
    Car& operator=(const Car&) = delete;

    explicit Car(uint32_t client_id, b2World& world, const ServerConfig& server_config);

    void handle_movement(const MovementDTO& movement_dto);

    void update_position();

    CarPositionDTO get_position_dto() const;

    void set_collision(bool colliding);
    b2Body* get_body() const;

    void set_configuration(const car_dto& car_configuration);
};

#endif
