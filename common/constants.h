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

};


// SERVER
constexpr int MAX_PATH_LENGTH = 4096;
constexpr char SERVER_CMD_EXIT = 'q';
constexpr std::string_view SERVER_STARTED = "SERVER STARTED";
constexpr std::string_view SERVER_STOPPED = "SERVER STOPPED";
constexpr std::string_view SERVER_CONFIG_FILEPATH = "server/server_config.yaml";

// SERVER CLASSES
constexpr std::string_view SERVER = "SERVER";
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
constexpr float PIXELS_PER_METER = 30.0f;
    // TODO: replace with command line parameter
constexpr std::string_view POLYGONS_LIBERTY_CITY = "server/world/polygons_liberty_city.json";

#endif
