#include <exception>
#include <iostream>

#include "client.h"

int main() try {
    Client client("127.0.0.1", "8080");
    client.run();
    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
