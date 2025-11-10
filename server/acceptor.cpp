#include "acceptor.h"

#include <iostream>
#include <utility>

#include "../common/liberror.h"


Acceptor::Acceptor(const std::string& port, GameMonitor& game_monitor):
        acceptor_socket(port.c_str()), game_monitor(game_monitor) {}


void Acceptor::run() {

    while (should_keep_running()) {

        try {

            Socket peer_socket = acceptor_socket.accept();
            reap();
            client_id++;
            std::unique_ptr<ClientHandler> client = std::make_unique<ClientHandler>(std::move(peer_socket), game_monitor,
                                                          client_id);
            clients.push_back(std::move(client));
            std::cout << "New client just connected! Number of active clients: " << clients.size() << std::endl;
        } catch (const std::exception& e) {

            if (!should_keep_running())
                break;
            std::cerr << "[" << ACCEPTOR << "] " << EXCEPTION << ": " << e.what();
        }
    }
}


void Acceptor::reap() {

    // lobby.reap();
    clients.remove_if([](const std::unique_ptr<ClientHandler>& client) {
        if (!client->is_alive()) {

            std::cout << "Reaping dead client..." << std::endl;
            return true;
        }

        return false;
    });
}


void Acceptor::stop() {

    _keep_running = false;
    try {

        acceptor_socket.shutdown(2);
        acceptor_socket.close();
    } catch (const LibError&) {}

    clients.clear();
}


Acceptor::~Acceptor() {

    try {

        Acceptor::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << ACCEPTOR << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
