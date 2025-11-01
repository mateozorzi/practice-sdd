#include "car.h"

#include <cmath>
#include "../common/constants.h"

Car::Car(b2World& world, const CarConfig& config): body(nullptr), config(config) {

    b2BodyDef body_definition;
    body_definition.type = b2_dynamicBody;
    body_definition.position.Set(0.0f, 0.0f);
    body_definition.angle = b2_pi / 2.0f;
    body_definition.linearDamping = config.linear_damping;
    body_definition.angularDamping = config.angular_damping;

    body = world.CreateBody(&body_definition);

    b2PolygonShape shape;
    shape.SetAsBox(config.width, config.height);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = config.density;
    fixture.friction = config.friction;
    fixture.restitution = config.restitution;

    body->CreateFixture(&fixture);
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

    b2Vec2 forward_vector = body->GetWorldVector(b2Vec2(0.0f, 1.0f));
    b2Vec2 reverse_vector = body->GetWorldVector(b2Vec2(0.0f, -1.0f));
    
    if (moving_up) {
        b2Vec2 force = -body->GetMass() * config.acceleration * forward_vector;
        body->ApplyForceToCenter(force, true);
        this->apply_torque(forward_vector);
    }

    if (moving_down) {
        b2Vec2 force = body->GetMass() * config.acceleration * config.braking_factor * forward_vector;
        body->ApplyForceToCenter(force, true);
        this->apply_torque(reverse_vector);
    }
}

void Car::apply_torque(b2Vec2 direction_vector) {
    b2Vec2 right_vector = body->GetWorldVector(b2Vec2(1.0f, 0.0f));
    b2Vec2 velocity = body->GetLinearVelocity();
    // float direction_speed = b2Dot(velocity, direction_vector);
    float lateral_speed = b2Dot(velocity, right_vector);

    float current_speed = b2Dot(body->GetLinearVelocity(), direction_vector);
    if (current_speed > config.max_speed) {
        b2Vec2 limited_velocity =
                config.max_speed * direction_vector +
                b2Dot(velocity, right_vector) * config.drift_factor * right_vector;
        body->SetLinearVelocity(limited_velocity);
    }

    b2Vec2 lateral_impulse = -lateral_speed * body->GetMass() * config.drift_factor * right_vector;
    body->ApplyLinearImpulseToCenter(lateral_impulse, true);

    if (moving_left 
        // && std::fabs(direction_speed) > config.min_turn_speed
    )
        body->ApplyTorque(-config.turn_torque, true);
    if (moving_right 
        // && std::fabs(direction_speed) > config.min_turn_speed
    )
        body->ApplyTorque(config.turn_torque, true);
}

CarPositionDTO Car::get_position_dto() const {

    b2Vec2 position = body->GetPosition();
    float angle_radians = body->GetAngle();

    uint32_t x_pixels = static_cast<uint32_t>(position.x * PIXELS_PER_METER);
    uint32_t y_pixels = static_cast<uint32_t>(position.y * PIXELS_PER_METER);
    uint32_t angle_degrees = static_cast<uint32_t>(angle_radians * (180.0f / b2_pi));
    return CarPositionDTO(x_pixels, y_pixels, angle_degrees);
}

void Car::set_collision(bool colliding) {
    is_colliding = colliding;
}

b2Body* Car::get_body() const {
    return body;
}
