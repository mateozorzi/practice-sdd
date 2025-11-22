#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <string_view>


enum class ClientAction : uint8_t {

    DISCONNECT = 0x00,
    SEND_USERNAME = 0x01,
    SEND_CAR_MOVEMENT = 0x03,
    GET_CAR_CATALOGUE = 0x04,
    SEND_CAR_SELECTION = 0x05,
    GET_JSON_FILE = 0x06,
    SEND_UPGRADE = 0x07,

};

enum class GameLoopAction : uint8_t {

    SEND_UPDATE_POSITIONS = 0x10,
    SEND_RACE = 0x11,
    SEND_RACE_FINISHED = 0x12,
    SEND_LEADERBOARD = 0x13,
    START_SHOP = 0x14,
    CLOSE_SHOP = 0x15,
};


// CHECKPOINTS
constexpr char HORIZONTAL = 'H';
constexpr char VERTICAL = 'V';
constexpr char INTERSECTION = 'I';

// SERVER
constexpr int MAX_PATH_LENGTH = 4096;
constexpr char SERVER_CMD_EXIT = 'q';
constexpr std::string_view SERVER_STARTED = "SERVER STARTED";
constexpr std::string_view SERVER_STOPPED = "SERVER STOPPED";
constexpr std::string_view SERVER_CONFIG_FILEPATH = "server/resources/server_config.yaml";

// SERVER CLASSES
constexpr std::string_view SERVER = "SERVER";
constexpr std::string_view LOBBY = "LOBBY";
constexpr std::string_view GAME = "GAME";
constexpr std::string_view GAME_MONITOR = "GAME_MONITOR";
constexpr std::string_view GAME_LOOP = "GAME_LOOP";
constexpr std::string_view ACCEPTOR = "ACCEPTOR";
constexpr std::string_view CLIENT_HANDLER = "CLIENT HANDLER";
constexpr std::string_view RECEIVER = "RECEIVER";
constexpr std::string_view SENDER = "SENDER";

// EXCEPTIONS
constexpr std::string_view EXCEPTION = "Exception";
constexpr std::string_view EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";

// WORLD
// TODO: remove, now in yaml
constexpr float PIXELS_PER_METER = 30.0f;
// TODO: replace with command line parameter
constexpr std::string_view POLYGONS_LIBERTY_CITY =
        "server/resources/world/blocks_liberty_city.json";


// cars
constexpr std::string_view CAR_COMPACT = "Compact";
constexpr std::string_view CAR_SPORT = "Sport";
constexpr std::string_view CAR_CONVERTIBLE = "Convertible";
constexpr std::string_view CAR_SEDAN = "Sedan";
constexpr std::string_view CAR_JEEP = "Jeep";
constexpr std::string_view CAR_RALLY = "Rally";
constexpr std::string_view CAR_TRUCK = "Truck";

#endif
