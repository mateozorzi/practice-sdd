#include <cstdint>
#include <iostream>
#include <string>

#include "server.h"
#include "server_config.h"
#include "config_loader.h"
#include "../common/constants.h"


constexpr uint8_t SUCCESS = 0;
constexpr uint8_t ERROR = 1;
constexpr uint8_t SERVER_PORT = 1;
constexpr uint8_t SERVER_ARGUMENTS = 2;

constexpr std::string_view SERVER_MAIN = "SERVER_MAIN";
constexpr std::string_view ERROR_ARGUMENTS_SERVER = "1 argument is needed: <service_name or port>";
constexpr std::string_view UNKNOWN_EXCEPTION = "Unknown exception";


// TODO: delete
void debug_print_config(ServerConfig config) {
    std::cout << "DEBUG CONFIG FILE" << "\n";
    std::cout << "app.game.game_fps: " << config.app.game.game_fps << "\n";
    std::cout << "app.server.world.pixels_per_meter: " << config.app.server.world.pixels_per_meter << "\n";

    std::cout << "app.server.world.collisions.liberty_city.blocks_file: " 
                << config.app.server.world.collisions.liberty_city.blocks_file << "\n";

    std::cout << "app.client.cars.compact.health: " << config.app.client.cars.compact.health << "\n";
}


int main(const int argc, const char* const argv[]) {

    if (argc != SERVER_ARGUMENTS) {

        std::cerr << "[" << SERVER_MAIN << "] " << ERROR_ARGUMENTS_SERVER << std::endl;
        return ERROR;
    }

    const std::string port = argv[SERVER_PORT];

    try {
        ServerConfig config = ConfigLoader::load_yaml(std::string(SERVER_CONFIG_FILEPATH));
        debug_print_config(config);

        Server server(port, config);
        server.run();
    } catch (const std::exception& e) {

        std::cerr << "[" << SERVER_MAIN << "] " << e.what() << std::endl;
        return ERROR;
    } catch (...) {

        std::cerr << "[" << SERVER_MAIN << "] " << UNKNOWN_EXCEPTION << std::endl;
        return ERROR;
    }

    return SUCCESS;
}
