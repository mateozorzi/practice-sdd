#ifndef CAR_H
#define CAR_H


#include <string>
#include <vector>

#include <box2d/box2d.h>

#include "../common/car_dto.h"
#include "../common/car_position_dto.h"
#include "../common/checkpoint_dto.h"
#include "../common/client_event_dto.h"
#include "../common/movement_dto.h"
#include "../common/upgrade_dto.h"

#include "server_config.h"


class Car {

    uint32_t client_id;
    b2Body* box2d_body;
    CarConfig config;
    std::string model;
    uint16_t base_health;

    bool moving_up = false;
    bool moving_down = false;
    bool moving_left = false;
    bool moving_right = false;
    bool is_colliding = false;
    CarLayer car_layer = CarLayer::ON_GROUND;

    void detect_linear_movement();
    void detect_rotation();
    void detect_drifting();

public:
    Car(const Car&) = delete;
    Car& operator=(const Car&) = delete;

    explicit Car(uint32_t client_id, b2World& box2d_world, const std::string& model,
                 const CarConfig& config);

    void set_position_for_race(const std::vector<CheckpointDTO>& checkpoints);

    void handle_movement(const MovementDTO& movement_dto);
    void update_position();
    CarPositionDTO get_position_dto() const;

    const CarDTO get_configuration() const;

    b2Body* get_body() const;

    CarLayer get_layer() const;

    // void upgrade_configuration(const UpgradeDTO& upgrade);

    void set_collision(bool colliding);
    void set_upgrade(const BuyUpgradeDTO& upgrade, const UpgradeConfig& game_config);

    void set_layer(CarLayer car_layer);
    void reset_car_velocity();
};

#endif
