#include "server.h"

#include <iostream>

#include "../common/destructor_log.h"


Server::Server(const std::string& port, const ServerConfig& server_config):
        server_config(server_config),
        lobby(server_config),
        clients_acceptor(port, lobby, server_config) {}


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

    lobby.stop();
}


Server::~Server() {
    log_stopping(SERVER);
    try {
        Server::stop();
    } catch (const std::exception& e) {
        log_destructor_error(SERVER, e.what());
    } catch (...) {
        log_destructor_error(SERVER, "<unknown>");
    }
}
