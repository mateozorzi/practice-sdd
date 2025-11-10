#include "server.h"

#include <iostream>


Server::Server(const std::string& port, ServerConfig config):
        config(config),
        game_monitor(),
        game_loop(game_monitor), 
        clients_acceptor(port, game_monitor/*lobby*/) {}


void Server::run() {

    try {

        game_loop.start();
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
    game_loop.stop();

    clients_acceptor.join();
    game_loop.join();
}


Server::~Server() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SERVER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
