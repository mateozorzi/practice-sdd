#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <string_view>


enum class ClientAction : uint8_t {

    DISCONNECT = 0x00,
    SEND_USERNAME = 0x01,
    SEND_CAR_MOVEMENT = 0x03,
    SEND_CAR_SELECTION = 0x05,
    GET_JSON_FILE = 0x06,
    SEND_UPGRADE = 0x07,

};

enum class GameLoopAction : uint8_t {

    SEND_UPDATE_POSITIONS = 0x10,
    SEND_RACE = 0x11,
    START_RACE = 0x12,
    SEND_LEADERBOARD = 0x13,
    START_SHOP = 0x14,
    START_COUNTDOWN = 0x15,
    GAME_FINISHED = 0x16,
    RETURN_TO_LOBBY = 0x17,

    CLOSE_SHOP = 0x18  // provisional, delete later
};


// COLLISIONS
constexpr uint16_t CATEGORY_GROUND = 0x0001;
constexpr uint16_t CATEGORY_BRIDGE = 0x0002;
constexpr uint16_t CATEGORY_CAR_GROUND = 0x0004;
constexpr uint16_t CATEGORY_CAR_BRIDGE = 0x0008;
constexpr uint16_t CATEGORY_CAR_UNDER = 0x0010;
constexpr uint16_t CATEGORY_SENSOR = CATEGORY_GROUND | CATEGORY_BRIDGE;

constexpr uint16_t MASK_GROUND = CATEGORY_GROUND | CATEGORY_CAR_GROUND | CATEGORY_CAR_UNDER;
constexpr uint16_t MASK_BRIDGE = CATEGORY_BRIDGE | CATEGORY_CAR_BRIDGE;
constexpr uint16_t MASK_CAR_GROUND = CATEGORY_GROUND | CATEGORY_CAR_GROUND | CATEGORY_CAR_UNDER;
constexpr uint16_t MASK_CAR_BRIDGE = CATEGORY_BRIDGE | CATEGORY_CAR_BRIDGE;
constexpr uint16_t MASK_CAR_UNDER = CATEGORY_GROUND | CATEGORY_CAR_GROUND | CATEGORY_CAR_UNDER;
constexpr uint16_t MASK_SENSOR = CATEGORY_CAR_GROUND | CATEGORY_CAR_BRIDGE | CATEGORY_CAR_UNDER;


// CHECKPOINTS
constexpr char HORIZONTAL = 'H';
constexpr char VERTICAL = 'V';
constexpr char INTERSECTION = 'I';

// SERVER
constexpr int MAX_PATH_LENGTH = 4096;

// EXCEPTIONS
constexpr std::string_view EXCEPTION = "Exception";
constexpr std::string_view EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";

// WORLD
// TODO: remove, now in yaml
constexpr float PIXELS_PER_METER = 30.0f;


#endif
