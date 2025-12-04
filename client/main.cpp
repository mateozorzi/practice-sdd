#include <exception>
#include <iostream>
#include <string>
#include <utility>

#include "client.h"
#include "menu_connection.h"

constexpr uint8_t SUCCESS = 0;
constexpr uint8_t ERROR = 1;

int main() try {
    MenuConnection menu_connection;
    std::pair<std::string, std::string> connection_data = menu_connection.run();
    if (connection_data.first.empty() || connection_data.second.empty()) {
        return SUCCESS;
    }
    Client client(connection_data.first, connection_data.second);
    client.run();
    return SUCCESS;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return ERROR;
}
