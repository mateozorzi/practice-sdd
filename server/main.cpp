#include <cstdint>
#include <iostream>
#include <string>

#include "../common/constants.h"

#include "server.h"
#include "server_config.h"
#include "server_config_loader.h"
#include "server_constants.h"


int main() {

    try {

        ServerConfig server_config =
                ServerConfigLoader().load_yaml(std::string(SERVER_CONFIG_FILEPATH));

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
