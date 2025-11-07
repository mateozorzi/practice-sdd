#ifndef MOVEMENT_DTO_H
#define MOVEMENT_DTO_H

enum class Direction { UP = 1, DOWN = 2, LEFT = 3, RIGHT = 4 };

struct MovementDTO {
    Direction direction;
    bool isMoving;

    explicit MovementDTO(Direction direction = Direction::UP, bool isMoving = false);
};

#endif
