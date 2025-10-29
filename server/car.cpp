#include "car.h"


Car::Car(b2World& world): body(nullptr), x(0), y(0), angle(0) {

    b2BodyDef body_definition;
    body_definition.type = b2_dynamicBody;
    body_definition.position.Set(0.0f, 0.0f);
    body_definition.linearDamping = 1.0f;
    body_definition.angularDamping = 2.0f;
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

bool Car::update_position() {

    const uint32_t speed = 2;

    if (moving_up && y > speed) {

        y -= speed;
        angle = 0;
    }
    if (moving_down) {

        y += speed;
        angle = 180;
    }
    if (moving_left && x > speed) {

        x -= speed;
        angle = 270;
    }
    if (moving_right) {

        x += speed;
        angle = 90;
    }

    return moving_up || moving_down || moving_left || moving_right;
}


CarPositionDTO Car::get_position_dto() const {

    return CarPositionDTO(x, y, angle);
}
