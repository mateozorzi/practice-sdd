#include "car.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "../common/constants.h"

Car::Car(uint32_t client_id, b2World& box2d_world, const std::string& model,
         const CarConfig& config):
        client_id(client_id), box2d_body(nullptr), config(config), model(model) {

    b2BodyDef box2d_body_definition;
    box2d_body_definition.type = b2_dynamicBody;
    box2d_body_definition.position.Set(0.0f, 0.0f);
    box2d_body_definition.angle = 0.0f;
    box2d_body_definition.linearDamping = config.linear_damping;
    box2d_body_definition.angularDamping = config.angular_damping;

    box2d_body = box2d_world.CreateBody(&box2d_body_definition);

    b2PolygonShape box2d_polygon_shape;
    box2d_polygon_shape.SetAsBox(config.width / 2.0f, config.height / 2.0f);

    b2FixtureDef box2d_fixture_definition;
    box2d_fixture_definition.shape = &box2d_polygon_shape;
    box2d_fixture_definition.density = config.density;
    box2d_fixture_definition.friction = config.friction;
    box2d_fixture_definition.restitution = config.restitution;

    b2Fixture* box2d_fixture = box2d_body->CreateFixture(&box2d_fixture_definition);

    b2Filter box2d_filter;
    box2d_filter.categoryBits = CATEGORY_CAR_GROUND;
    box2d_filter.maskBits = MASK_CAR_GROUND;
    box2d_fixture->SetFilterData(box2d_filter);

    // Set mass explicitly
    b2MassData mass_data;
    box2d_body->GetMassData(&mass_data);
    mass_data.mass = config.mass;
    box2d_body->SetMassData(&mass_data);
}


void Car::set_position_for_race(const std::vector<CheckpointDTO>& checkpoints) {
    CheckpointDTO start_position = checkpoints[0];
    CheckpointDTO second_position = checkpoints[1];

    // cars are placed in the first checkpoint, but facing the second one

    float start_x = static_cast<float>(start_position.x) / PIXELS_PER_METER;
    float start_y = static_cast<float>(start_position.y) / PIXELS_PER_METER;
    float angle = 0.0f;

    // first checkpoint is horizontal
    if (start_position.orientation == HORIZONTAL) {
        // second checkpoint is above or below

        if (second_position.y >= start_position.y) {  // above
            angle = b2_pi;
            start_y -= 1.0f;
        } else {  // below
            angle = 0.0f;
            start_y += 1.0f;
        }
    } else if (start_position.orientation == VERTICAL) {  // first checkpoint is vertical
        // second checkpoint is to the left or right
        if (second_position.x <= start_position.x) {  // left
            angle = 3.0f * b2_pi / 2.0f;
            start_x += 1.0f;
        } else {  // right
            angle = b2_pi / 2.0f;
            start_x -= 1.0f;
        }
    } else {                                         // first checkpoint in intersection
        if (second_position.y > start_position.y) {  // above
            angle = b2_pi;
            start_y -= 1.0f;
        } else if (second_position.y < start_position.y) {  // below
            angle = 0.0f;
            start_y += 1.0f;
        } else {                                         // same y, must be left or right
            if (second_position.x < start_position.x) {  // left
                angle = 3.0f * b2_pi / 2.0f;
                start_x += 1.0f;
            } else {  // right
                angle = b2_pi / 2.0f;
                start_x -= 1.0f;
            }
        }
    }

    box2d_body->SetTransform(b2Vec2(start_x, start_y), angle);
}


void Car::detect_linear_movement() {

    b2Vec2 forward_vector = box2d_body->GetWorldVector(b2Vec2(0.0f, 1.0f));

    b2Vec2 force = box2d_body->GetMass() * config.acceleration * forward_vector;

    if (moving_up)
        box2d_body->ApplyForceToCenter(-force, true);

    if (moving_down)
        box2d_body->ApplyForceToCenter(config.braking_factor * force, true);

    b2Vec2 velocity = box2d_body->GetLinearVelocity();
    float current_speed = velocity.Length();

    if (current_speed > config.max_speed) {

        float scale = config.max_speed / current_speed;
        b2Vec2 limited_velocity(scale * velocity);
        box2d_body->SetLinearVelocity(limited_velocity);
    }
}

void Car::detect_rotation() {

    b2Vec2 velocity = box2d_body->GetLinearVelocity();
    float current_speed = velocity.Length();

    if (current_speed > config.min_turn_speed) {

        if (moving_left)
            box2d_body->ApplyTorque(-config.turn_torque, true);
        if (moving_right)
            box2d_body->ApplyTorque(config.turn_torque, true);
    }
}

void Car::detect_drifting() {

    b2Vec2 right_vector = box2d_body->GetWorldVector(b2Vec2(1.0f, 0.0f));
    b2Vec2 velocity = box2d_body->GetLinearVelocity();

    b2Vec2 lateral_velocity = b2Dot(velocity, right_vector) * right_vector;
    b2Vec2 lateral_impulse = -config.drift_factor * box2d_body->GetMass() * lateral_velocity;

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

CarPositionDTO Car::get_position_dto() const {

    b2Vec2 position_meters = box2d_body->GetPosition();
    float angle_radians = box2d_body->GetAngle();

    uint32_t x_pixels = static_cast<uint32_t>(position_meters.x * PIXELS_PER_METER);
    uint32_t y_pixels = static_cast<uint32_t>(position_meters.y * PIXELS_PER_METER);

    float angle_degrees_float = angle_radians * (180.0f / b2_pi);

    angle_degrees_float = fmod(angle_degrees_float, 360.0f);
    if (angle_degrees_float < 0.0f)
        angle_degrees_float += 360.0f;

    uint32_t angle_degrees = static_cast<uint32_t>(angle_degrees_float + 0.5f);

    if (angle_degrees >= 360)
        angle_degrees = 0;

    return CarPositionDTO(client_id, x_pixels, y_pixels, angle_degrees, CheckpointDTO(), car_layer);
}


const CarDTO Car::get_configuration() const {

    CarDTO car_configuration(model);
    return car_configuration;
}


b2Body* Car::get_body() const { return box2d_body; }


CarLayer Car::get_layer() const { return car_layer; }


void Car::set_collision(bool colliding) { is_colliding = colliding; }


void Car::set_upgrade(const BuyUpgradeDTO& upgrade, const UpgradeConfig& game_config) {

    if (upgrade.upgrade == ButtonType::Speed) {

        std::cout << "Upgrading speed for player id: " << client_id << std::endl;
        std::cout << "Current speed: " << config.max_speed << std::endl;
        config.max_speed += static_cast<float>(game_config.speed_upgrade);
        std::cout << "New speed: " << config.max_speed << std::endl;
    } else if (upgrade.upgrade == ButtonType::Acceleration) {

        std::cout << "Upgrading acceleration for player id: " << client_id << std::endl;
        std::cout << "Current acceleration: " << config.acceleration << std::endl;
        config.acceleration += static_cast<float>(game_config.acceleration_upgrade);
        std::cout << "New acceleration: " << config.acceleration << std::endl;
    } else if (upgrade.upgrade == ButtonType::Handling) {

        std::cout << "Upgrading handling for player id: " << client_id << std::endl;
        std::cout << "Current turn_torque: " << config.turn_torque << std::endl;
        std::cout << "Current drift_factor: " << config.drift_factor << std::endl;

        config.turn_torque += static_cast<float>(game_config.handling_upgrade) * 0.0001f;
        config.drift_factor = std::max(0.0f, config.drift_factor - 0.05f);

        std::cout << "New turn_torque: " << config.turn_torque << std::endl;
        std::cout << "New drift_factor: " << config.drift_factor << std::endl;
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
    box2d_body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    box2d_body->SetAngularVelocity(0.0f);
    box2d_body->SetAwake(false);
}
