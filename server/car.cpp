#include "car.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "../common/constants.h"

Car::Car(uint32_t player_id, b2World& box2d_world, const std::string& model,
         const ServerConfig& server_config):
        player_id(player_id),
        box2d_body(nullptr),
        server_config(server_config),
        car_config(find_car_config(model, server_config.physics)),
        base_health(car_config.health) {


    b2BodyDef box2d_body_definition;
    box2d_body_definition.type = b2_dynamicBody;
    box2d_body_definition.position.Set(0.0f, 0.0f);
    box2d_body_definition.angle = 0.0f;
    box2d_body_definition.linearDamping = car_config.linear_damping;
    box2d_body_definition.angularDamping = car_config.angular_damping;

    box2d_body = box2d_world.CreateBody(&box2d_body_definition);

    b2PolygonShape box2d_polygon_shape;
    box2d_polygon_shape.SetAsBox(car_config.width / 2.0f, car_config.height / 2.0f);

    b2FixtureDef box2d_fixture_definition;
    box2d_fixture_definition.shape = &box2d_polygon_shape;
    box2d_fixture_definition.density = car_config.density;
    box2d_fixture_definition.friction = car_config.friction;
    box2d_fixture_definition.restitution = car_config.restitution;

    b2Fixture* box2d_fixture = box2d_body->CreateFixture(&box2d_fixture_definition);

    b2Filter box2d_filter;
    box2d_filter.categoryBits = CATEGORY_CAR_GROUND;
    box2d_filter.maskBits = MASK_CAR_GROUND;
    box2d_fixture->SetFilterData(box2d_filter);

    // Set mass explicitly
    b2MassData mass_data;
    box2d_body->GetMassData(&mass_data);
    mass_data.mass = car_config.mass;
    box2d_body->SetMassData(&mass_data);
}


const CarConfig& Car::find_car_config(const std::string& model, const PhysicsConfig& physics) {

    if (model == physics.compact.model)
        return physics.compact;
    if (model == physics.sport.model)
        return physics.sport;
    if (model == physics.convertible.model)
        return physics.convertible;
    if (model == physics.sedan.model)
        return physics.sedan;
    if (model == physics.jeep.model)
        return physics.jeep;
    if (model == physics.rally.model)
        return physics.rally;
    if (model == physics.truck.model)
        return physics.truck;

    return physics.compact;
}


void Car::set_position_for_race(const std::vector<CheckpointDTO>& checkpoints) {
    CheckpointDTO start_position = checkpoints[0];
    CheckpointDTO second_position = checkpoints[1];

    float start_x = static_cast<float>(start_position.x) / server_config.world.pixels_per_meter;
    float start_y = static_cast<float>(start_position.y) / server_config.world.pixels_per_meter;
    float angle = 0.0f;

    if (start_position.orientation == HORIZONTAL) {

        if (second_position.y >= start_position.y) {
            angle = b2_pi;
            start_y -= 1.0f;
        } else {
            angle = 0.0f;
            start_y += 1.0f;
        }
    } else if (start_position.orientation == VERTICAL) {

        if (second_position.x <= start_position.x) {
            angle = 3.0f * b2_pi / 2.0f;
            start_x += 1.0f;
        } else {
            angle = b2_pi / 2.0f;
            start_x -= 1.0f;
        }
    } else {
        if (second_position.y > start_position.y) {
            angle = b2_pi;
            start_y -= 1.0f;
        } else if (second_position.y < start_position.y) {
            angle = 0.0f;
            start_y += 1.0f;
        } else {
            if (second_position.x < start_position.x) {
                angle = 3.0f * b2_pi / 2.0f;
                start_x += 1.0f;
            } else {
                angle = b2_pi / 2.0f;
                start_x -= 1.0f;
            }
        }
    }

    box2d_body->SetTransform(b2Vec2(start_x, start_y), angle);
}


void Car::detect_linear_movement() {

    b2Vec2 forward_vector = box2d_body->GetWorldVector(b2Vec2(0.0f, 1.0f));

    b2Vec2 force = box2d_body->GetMass() * car_config.acceleration * forward_vector;

    if (moving_up)
        box2d_body->ApplyForceToCenter(-force, true);

    if (moving_down)
        box2d_body->ApplyForceToCenter(car_config.braking_factor * force, true);

    b2Vec2 velocity = box2d_body->GetLinearVelocity();
    float current_speed = velocity.Length();

    if (current_speed > car_config.max_speed) {

        float scale = car_config.max_speed / current_speed;
        b2Vec2 limited_velocity(scale * velocity);
        box2d_body->SetLinearVelocity(limited_velocity);
    }
}

void Car::detect_rotation() {

    b2Vec2 velocity = box2d_body->GetLinearVelocity();
    float current_speed = velocity.Length();

    if (current_speed > car_config.min_turn_speed) {

        if (moving_left)
            box2d_body->ApplyTorque(-car_config.turn_torque, true);
        if (moving_right)
            box2d_body->ApplyTorque(car_config.turn_torque, true);
    }
}

void Car::detect_drifting() {

    b2Vec2 right_vector = box2d_body->GetWorldVector(b2Vec2(1.0f, 0.0f));
    b2Vec2 velocity = box2d_body->GetLinearVelocity();

    b2Vec2 lateral_velocity = b2Dot(velocity, right_vector) * right_vector;
    b2Vec2 lateral_impulse = -car_config.drift_factor * box2d_body->GetMass() * lateral_velocity;

    box2d_body->ApplyLinearImpulseToCenter(lateral_impulse, true);
}


void Car::handle_movement(const MovementDTO& movement_dto) {

    switch (movement_dto.direction) {

        case Direction::UP:
            moving_up = movement_dto.isMoving;
            break;
        case Direction::DOWN:
            moving_down = movement_dto.isMoving;
            break;
        case Direction::LEFT:
            moving_left = movement_dto.isMoving;
            break;
        case Direction::RIGHT:
            moving_right = movement_dto.isMoving;
            break;
    }
}

void Car::update_position() {

    detect_linear_movement();

    detect_rotation();

    detect_drifting();
}


uint16_t Car::get_max_health() const { return car_config.health; }


CarPositionDTO Car::get_position_dto() const {

    b2Vec2 position_meters = box2d_body->GetPosition();
    float angle_radians = box2d_body->GetAngle();

    uint32_t x_pixels =
            static_cast<uint32_t>(position_meters.x * server_config.world.pixels_per_meter);
    uint32_t y_pixels =
            static_cast<uint32_t>(position_meters.y * server_config.world.pixels_per_meter);

    float angle_degrees_float = angle_radians * (180.0f / b2_pi);

    angle_degrees_float = fmod(angle_degrees_float, 360.0f);
    if (angle_degrees_float < 0.0f)
        angle_degrees_float += 360.0f;

    uint32_t angle_degrees = static_cast<uint32_t>(angle_degrees_float + 0.5f);

    if (angle_degrees >= 360)
        angle_degrees = 0;

    return CarPositionDTO(player_id, x_pixels, y_pixels, angle_degrees, CheckpointDTO(), car_layer);
}


const CarDTO Car::get_configuration() const {

    CarDTO car_configuration(car_config.model, base_health, 0, 0, 0);
    return car_configuration;
}


b2Body* Car::get_body() const { return box2d_body; }


CarLayer Car::get_layer() const { return car_layer; }


void Car::set_collision(bool colliding) { is_colliding = colliding; }


void Car::set_upgrade(const BuyUpgradeDTO& upgrade, const UpgradeConfig& game_config) {

    if (upgrade.upgrade == ButtonType::Speed) {

        car_config.max_speed += static_cast<float>(game_config.speed_upgrade);
    } else if (upgrade.upgrade == ButtonType::Acceleration) {

        car_config.acceleration += static_cast<float>(game_config.acceleration_upgrade);
    } else if (upgrade.upgrade == ButtonType::Handling) {

        car_config.turn_torque += static_cast<float>(game_config.handling_upgrade) * 0.0001f;
        car_config.drift_factor = std::max(0.0f, car_config.drift_factor - 0.05f);
    }
}


void Car::set_layer(CarLayer layer) {

    car_layer = layer;

    b2Fixture* fixture = box2d_body->GetFixtureList();

    if (fixture) {

        b2Filter filter = fixture->GetFilterData();
        switch (layer) {

            case CarLayer::ON_GROUND:
                filter.categoryBits = CATEGORY_CAR_GROUND;
                filter.maskBits = MASK_CAR_GROUND;
                break;
            case CarLayer::ON_BRIDGE:
                filter.categoryBits = CATEGORY_CAR_BRIDGE;
                filter.maskBits = MASK_CAR_BRIDGE;
                break;
            case CarLayer::UNDER_BRIDGE:
                filter.categoryBits = CATEGORY_CAR_UNDER;
                filter.maskBits = MASK_CAR_UNDER;
                break;
        }

        fixture->SetFilterData(filter);
    }
}

void Car::reset_car_velocity() {
    moving_up = false;
    moving_down = false;
    moving_left = false;
    moving_right = false;
    box2d_body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    box2d_body->SetAngularVelocity(0.0f);
    box2d_body->SetAwake(false);
}
