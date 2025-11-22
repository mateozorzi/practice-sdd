#ifndef CAR_H
#define CAR_H


#include <box2d/box2d.h>

#include "../common/car_dto.h"
#include "../common/car_position_dto.h"
#include "../common/movement_dto.h"
#include "../common/checkpoint_dto.h"
#include "server_config.h"
#include <vector>
#include "../common/upgrade_dto.h"
#include "../common/client_event_dto.h"

// Collision filtering
constexpr uint16_t CATEGORY_GROUND = 0x0001;
constexpr uint16_t CATEGORY_BRIDGE = 0x0002;
constexpr uint16_t CATEGORY_CAR_GROUND = 0x0004;
constexpr uint16_t CATEGORY_CAR_BRIDGE = 0x0008; 
constexpr uint16_t CATEGORY_CAR_UNDER = 0x0010;
constexpr uint16_t CATEGORY_SENSOR = CATEGORY_GROUND | CATEGORY_BRIDGE;

constexpr uint16_t MASK_GROUND = CATEGORY_GROUND | CATEGORY_CAR_GROUND | CATEGORY_CAR_UNDER;
constexpr uint16_t MASK_BRIDGE = CATEGORY_BRIDGE | CATEGORY_CAR_BRIDGE;
constexpr uint16_t MASK_CAR_GROUND = CATEGORY_GROUND | CATEGORY_CAR_GROUND | CATEGORY_CAR_UNDER;
constexpr uint16_t MASK_CAR_BRIDGE = CATEGORY_BRIDGE | CATEGORY_CAR_BRIDGE;
constexpr uint16_t MASK_CAR_UNDER = CATEGORY_GROUND | CATEGORY_CAR_GROUND | CATEGORY_CAR_UNDER;
constexpr uint16_t MASK_SENSOR = CATEGORY_CAR_GROUND | CATEGORY_CAR_BRIDGE | CATEGORY_CAR_UNDER;


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
    std::string model;

    bool moving_up = false;
    bool moving_down = false;
    bool moving_left = false;
    bool moving_right = false;
    bool is_colliding = false;
    CarLayer car_layer = CarLayer::ON_GROUND;
    uint8_t handling;

    float remap(float value, float inMin, float inMax, float outMin, float outMax);

    void detect_linear_movement();
    void detect_rotation();
    void detect_drifting();

public:
    Car(const Car&) = delete;
    Car& operator=(const Car&) = delete;

    explicit Car(uint32_t client_id, b2World& world, const ServerConfig& server_config);

    void set_position_for_race(const std::vector<CheckpointDTO>& checkpoints);

    void handle_movement(const MovementDTO& movement_dto);
    void update_position();
    CarPositionDTO get_position_dto() const;

    const CarDTO get_configuration() const;

    b2Body* get_body() const;

    CarLayer get_layer() const;

    void upgrade_configuration(const UpgradeDTO& upgrade);

    void set_collision(bool colliding);
    void set_configuration(const CarDTO& car_configuration, const CarClientConfig& car_config);
    void set_upgrade(const BuyUpgradeDTO& upgrade, const UpgradeConfig& game_config);

    void set_layer(CarLayer car_layer);
    void reset_car_velocity();
};

#endif
