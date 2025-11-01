#ifndef CAR_H
#define CAR_H


#include <box2d/box2d.h>

#include "../common/car_position_dto.h"
#include "../common/movement_dto.h"


struct CarConfig {

    float width = 1.0f;
    float height = 0.5f;
    float density = 1.0f;
    float friction = 0.4f;
    float restitution = 0.0f;

    float linear_damping = 1.0f;   // Sliding effect after stopping moving (lower = more sliding)
    float angular_damping = 2.5f;  // The same that above, but when turning (lower = more turning)
    float min_turn_speed = 1.3f;

    float acceleration = 4.0f;
    float braking_factor = 0.2f;  // How strong the car brakes
    float max_speed = 14.0f;
    float drift_factor = 0.7f;  // The lower = more drifting
    float turn_torque = 4.8f;

    float mass = 1.0f;
    int health = 100;
};


class Car {

    b2Body* body;
    CarConfig config;

    bool moving_up = false;
    bool moving_down = false;
    bool moving_left = false;
    bool moving_right = false;
    bool is_colliding = false;


public:
    Car(const Car&) = delete;
    Car& operator=(const Car&) = delete;

    explicit Car(b2World& world, const CarConfig& config);

    void handle_movement(const MovementDTO& movement_dto);
    void update_position();

    CarPositionDTO get_position_dto() const;
    
    void set_collision(bool colliding);
    b2Body* get_body() const;
};

#endif
