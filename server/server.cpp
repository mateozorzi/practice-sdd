#include "server.h"


#include <iostream>


Server::Server(const std::string& port) {

    (void)port; // For now, to avoid compiling errors!
}


void Server::run() {

    try {

        std::cout << SERVER_STARTED << std::endl;

        while (std::cin.get() != SERVER_CMD_EXIT) {}

        std::cout << SERVER_STOPPED << std::endl;

    } catch (const std::exception& e) {

        std::cerr << "[" << SERVER << "] " << EXCEPTION << ": " << e.what();
    }
}


void Server::stop() {

    // No threads to stop & join for now...
}


Server::~Server() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SERVER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
