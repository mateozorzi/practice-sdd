#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <string_view>


enum class ClientAction : uint8_t { SEND_USERNAME = 0x01, SEND_CAR_MOVEMENT = 0x03 };


// SERVER
constexpr char SERVER_CMD_EXIT = 'q';
constexpr std::string_view SERVER_STARTED = "SERVER STARTED";
constexpr std::string_view SERVER_STOPPED = "SERVER STOPPED";

// PROTOCOL
constexpr uint8_t DISCONNECT = 0x00;
constexpr uint8_t SEND_USERNAME = 0x01;
constexpr uint8_t MOVE_CAR = 0x03;

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
// TODO: replace with command line parameter
constexpr std::string_view POLYGONS_LIBERTY_CITY = "/home/nestor/Desktop/CLIonProjects/TP_Taller_de_Programacion-Need-For-Speed/server/world/polygons_liberty_city.json";


#endif
