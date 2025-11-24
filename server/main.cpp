#include <cstdint>
#include <iostream>
#include <string>

#include "../common/constants.h"

#include "config_loader.h"
#include "server.h"
#include "server_config.h"


constexpr uint8_t SUCCESS = 0;
constexpr uint8_t ERROR = 1;

constexpr std::string_view SERVER_MAIN = "SERVER_MAIN";
constexpr std::string_view UNKNOWN_EXCEPTION = "Unknown exception";


int main() {

    try {
        ServerConfig server_config = ConfigLoader().load_yaml(std::string(SERVER_CONFIG_FILEPATH));

        const std::string port = std::to_string(server_config.server.server_port);
        Server server(port, server_config);
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
