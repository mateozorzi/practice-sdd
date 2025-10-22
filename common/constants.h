#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

enum class CarMovement : uint8_t {
    ACCELERATE = 0x01,
    BRAKE = 0x02,
    TURN_LEFT = 0x03,
    TURN_RIGHT = 0x04
};

enum class ClientAction : uint8_t {
    SEND_USERNAME = 0x01,
    SEND_CAR_MOVEMENT = 0x03
};

#endif
