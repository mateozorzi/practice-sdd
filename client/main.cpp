#include <exception>
#include <iostream>

#include "client.h"

constexpr uint8_t SUCCESS = 0;
constexpr uint8_t ERROR = 1;

int main() try {
    Client client("127.0.0.1", "8080");
    client.run();
    return SUCCESS;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return ERROR;
}
