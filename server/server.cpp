#include "server.h"

#include <iostream>


Server::Server(const std::string& port, ServerConfig config):
        config(config), clients_acceptor(port, lobby) {}


void Server::run() {

    try {

        clients_acceptor.start();
        std::cout << SERVER_STARTED << std::endl;

        while (std::cin.get() != SERVER_CMD_EXIT) {}

        std::cout << SERVER_STOPPED << std::endl;

    } catch (const std::exception& e) {

        std::cerr << "[" << SERVER << "] " << EXCEPTION << ": " << e.what();
    }
}


void Server::stop() {

    clients_acceptor.stop();

    clients_acceptor.join();
}


Server::~Server() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SERVER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
