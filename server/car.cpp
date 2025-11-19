#include "car.h"

#include <cmath>
#include <iostream>

#include "../common/constants.h"
#include "../common/checkpoint_dto.h"

Car::Car(const uint32_t client_id, b2World& world, const ServerConfig& server_config):
        client_id(client_id), body(nullptr), config({
            .width = server_config.app.server.world.sprites.car.width,
            .height = server_config.app.server.world.sprites.car.height,
            .density = server_config.app.server.world.physics.car.density,
            .friction = server_config.app.server.world.physics.car.friction,
            .restitution = server_config.app.server.world.physics.car.restitution,
            .linear_damping = server_config.app.server.world.physics.car.linear_damping,
            .angular_damping = server_config.app.server.world.physics.car.angular_damping,
            .min_turn_speed = server_config.app.server.world.physics.car.min_turn_speed,
            .acceleration = server_config.app.server.world.physics.car.acceleration,
            .breaking_factor = server_config.app.server.world.physics.car.breaking_factor,
            .max_speed = server_config.app.server.world.physics.car.max_speed,
            .drift_factor = server_config.app.server.world.physics.car.drift_factor,
            .turn_torque = server_config.app.server.world.physics.car.turn_torque,
            .mass = server_config.app.server.world.physics.car.mass,
            .health = server_config.app.server.world.physics.car.health
        }) {

    b2BodyDef body_definition;
    body_definition.type = b2_dynamicBody;
    body_definition.position.Set(
        server_config.app.server.world.positions.race_start.meters_x,
        server_config.app.server.world.positions.race_start.meters_y
    );
    body_definition.angle = b2_pi / 2.0f;
    body_definition.linearDamping = config.linear_damping;
    body_definition.angularDamping = config.angular_damping;

    body = world.CreateBody(&body_definition);

    b2PolygonShape shape;
    shape.SetAsBox(config.width / 2.0f, config.height / 2.0f);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = config.density;
    fixture.friction = config.friction;
    fixture.restitution = config.restitution;

    b2Fixture* car_fixture = body->CreateFixture(&fixture);

    b2Filter filter;
    filter.categoryBits = CATEGORY_CAR_GROUND;
    filter.maskBits = MASK_CAR_GROUND;
    car_fixture->SetFilterData(filter);
}

void Car::set_position_for_race(const std::vector<CheckpointDTO>& checkpoints) {
    CheckpointDTO start_position = checkpoints[0];
    CheckpointDTO second_position = checkpoints[1];

    //cars are placed in the first checkpoint, but facing the second one
    
    float start_x = static_cast<float>(start_position.x) / PIXELS_PER_METER;
    float start_y = static_cast<float>(start_position.y) / PIXELS_PER_METER;
    float angle = 0.0f;

    //first checkpoint is horizontal
    if (start_position.orientation == HORIZONTAL) {
        //second checkpoint is above or below

        if (second_position.y >= start_position.y) { //above
            angle = b2_pi;
            start_y -= 1.0f;
        } else { //below
            angle = 0.0f;
            start_y += 1.0f;
        }
    } else if (start_position.orientation == VERTICAL) { //first checkpoint is vertical
        //second checkpoint is to the left or right
        if (second_position.x <= start_position.x) { //left
            angle = 3.0f * b2_pi / 2.0f;
            start_x += 1.0f;
        } else { //right
            angle = b2_pi / 2.0f;
            start_x -= 1.0f;
        }
    } else { //first checkpoint in intersection
        if (second_position.y > start_position.y) { //above
            angle = b2_pi;
            start_y -= 1.0f;
        } else if (second_position.y < start_position.y) { //below
            angle = 0.0f;
            start_y += 1.0f;
        } else {//same y, must be left or right
            if (second_position.x < start_position.x) { //left
                angle = 3.0f * b2_pi / 2.0f;
                start_x += 1.0f;
            } else { //right
                angle = b2_pi / 2.0f;
                start_x -= 1.0f;
            }
        }
    }

    body->SetTransform(
        b2Vec2(
            start_x,
            start_y
        ),
        angle
    );
    
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


void Car::detect_linear_movement() {

    b2Vec2 forward_vector = body->GetWorldVector(b2Vec2(0.0f, 1.0f));

    b2Vec2 force = body->GetMass() * config.acceleration * forward_vector;

    if (moving_up)
        body->ApplyForceToCenter(-force, true);

    if (moving_down)
        body->ApplyForceToCenter(config.breaking_factor * force, true);

    b2Vec2 velocity = body->GetLinearVelocity();
    float current_speed = velocity.Length();

    if (current_speed > config.max_speed) {

        float scale = config.max_speed / current_speed;
        b2Vec2 limited_velocity(velocity.x * scale, velocity.y * scale);
        body->SetLinearVelocity(limited_velocity);
    }
}

void Car::detect_rotation() {

    b2Vec2 velocity = body->GetLinearVelocity();
    float current_speed = velocity.Length();

    if (current_speed > config.min_turn_speed) {

        if (moving_left)
            body->ApplyTorque(-config.turn_torque, true);
        if (moving_right)
            body->ApplyTorque(config.turn_torque, true);
    }
}

void Car::detect_drifting() {

    b2Vec2 right_vector = body->GetWorldVector(b2Vec2(1.0f, 0.0f));
    b2Vec2 velocity = body->GetLinearVelocity();

    b2Vec2 lateral_velocity = b2Dot(velocity, right_vector) * right_vector;
    b2Vec2 lateral_impulse = -config.drift_factor * body->GetMass() * lateral_velocity;

    body->ApplyLinearImpulseToCenter(lateral_impulse, true);
}


void Car::update_position() {

    detect_linear_movement();

    detect_rotation();

    detect_drifting();
}

CarPositionDTO Car::get_position_dto() const {

    b2Vec2 position_meters = body->GetPosition();
    float angle_radians = body->GetAngle();

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


void Car::set_collision(bool colliding) { is_colliding = colliding; }

b2Body* Car::get_body() const { return body; }


const car_dto Car::get_configuration() const {
    car_dto car_configuration(model);
    return car_configuration;
}

float remap(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

void Car::set_configuration(const car_dto& car_configuration, const CarClientConfig& car) {
    std::cout<<"Setting car configuration for player id: "<<client_id<<" model: "<<car_configuration.model<<std::endl;
    model = car_configuration.model;

    config.max_speed = car.speed;
    config.acceleration = car.acceleration;
    config.turn_torque = remap(car.handling, 30.0f, 100.0f, 0.010f, 0.028f);
    std::cout<<"Turn torque set to: "<<config.turn_torque<<std::endl;
    config.drift_factor = remap(car.handling, 30.0f, 100.0f, 0.40f, 0.00f);
    std::cout<<"Drift factor set to: "<<config.drift_factor<<std::endl;
    //0.05
    // Need to check which model of car the client wants here.
    //(void)car_configuration;  // To avoid unused parameter warning
}

CarLayer Car::get_layer() const {

    return car_layer;
}

void Car::set_layer(CarLayer layer) {

    car_layer = layer;

    b2Fixture* fixture = body->GetFixtureList();

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
