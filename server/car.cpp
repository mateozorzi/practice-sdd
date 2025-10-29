#include "car.h"


Car::Car(b2World& world): body(nullptr) {

    b2BodyDef body_definition;
    body_definition.type = b2_dynamicBody;
    body_definition.position.Set(0.0f, 0.0f);
    body_definition.linearDamping = 2.0f;
    body_definition.angularDamping = 4.0f;
    body = world.CreateBody(&body_definition);

    b2PolygonShape shape;
    shape.SetAsBox(1.0f, 0.5f);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = 1.0f;
    fixture.friction = 0.3f;
    fixture.restitution = 0.0f;

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

void Car::update_position(float delta_time) {

    b2Vec2 forward_vector = body->GetWorldVector(b2Vec2(0.0f, 1.0f));
    b2Vec2 right_vector = body->GetWorldVector(b2Vec2(1.0f, 0.0f));

    b2Vec2 velocity = body->GetLinearVelocity();

    float forward_speed = b2Dot(velocity, forward_vector);
    float lateral_speed = b2Dot(velocity, right_vector);

    // 0.85f = DRIFT FACTOR
    b2Vec2 new_velocity = forward_speed * forward_vector + (lateral_speed * 0.85f) * right_vector;

    body->SetLinearVelocity(new_velocity);

    // 10.0f = ACCELERATION
    if (moving_up)
        forward_speed -= 10.0f * delta_time;

    if (moving_down)
        forward_speed += 10.0f * 0.06f * delta_time;

    // 15.0f = MAX_SPEED
    if (forward_speed > 15.0f)
        forward_speed = 15.0f;
    else if (forward_speed < -15.0f)
        forward_speed = -15.0f;
    
    body->SetLinearVelocity(forward_speed * forward_vector + (lateral_speed * 0.85) * right_vector);

    // 5.0f = TURN_TORQUE
    if (moving_left)
        body->ApplyTorque(-5.0f, true);
    if (moving_right)
        body->ApplyTorque(5.0f, true);
}


CarPositionDTO Car::get_position_dto() const {

    b2Vec2 position = body->GetPosition();
    float angle_radians = body->GetAngle();

    // 20.0f = PIXELS_PER_METER
    uint32_t x_pixels = static_cast<uint32_t>(position.x * 20.0f);
    uint32_t y_pixels = static_cast<uint32_t>(position.y * 20.0f);
    uint32_t angle_degrees = static_cast<uint32_t>(angle_radians * (180.0f / 3.14159265f));
    return CarPositionDTO(x_pixels, y_pixels, angle_degrees);
}
