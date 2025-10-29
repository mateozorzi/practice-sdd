#include "car.h"


Car::Car() : x(300), y(200), angle(0),
    moving_up(false), moving_down(false),
    moving_left(false), moving_right(false) {}


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
