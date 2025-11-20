#ifndef EVENT_DTO_H
#define EVENT_DTO_H
#include <variant>

enum class Direction { UP = 1, DOWN = 2, LEFT = 3, RIGHT = 4 };

struct MovementDTO {
    Direction direction;
    bool isMoving;
};

struct BuyUpgradeDTO {
    const ButtonType upgrade;
};

using ClientEventDTO = std::variant<MovementDTO, BuyUpgradeDTO>;
#endif