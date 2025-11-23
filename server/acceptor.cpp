#include "acceptor.h"

#include <iostream>
#include <utility>

#include "../common/liberror.h"


Acceptor::Acceptor(const std::string& port, Lobby& lobby, const ServerConfig& server_config):
        acceptor_socket(port.c_str()), lobby(lobby), server_config(server_config) {}


void Acceptor::run() {

    while (should_keep_running()) {

        try {

            Socket peer_socket = acceptor_socket.accept();
            reap();
            client_id++;
            std::unique_ptr<ClientHandler> client = std::make_unique<ClientHandler>(
                    std::move(peer_socket), lobby, client_id, server_config);
            client->start();
            clients.push_back(std::move(client));
            std::cout << "[" << ACCEPTOR << "] "
                      << "New client has been accepted. Number of active clients: "
                      << clients.size() << std::endl;
        } catch (const std::exception& e) {

            if (!should_keep_running())
                break;
            std::cerr << "[" << ACCEPTOR << "] " << EXCEPTION << ": " << e.what();
        }
    }
}


void Acceptor::reap() {

    clients.remove_if([](const std::unique_ptr<ClientHandler>& client) {
        if (!client->is_alive()) {

            client->join();
            std::cout << "[" << ACCEPTOR << "] "
                      << "Reaping dead client." << std::endl;
            return true;
        }

        return false;
    });

    lobby.reap();
}

void Acceptor::clear() {

    for (auto& client: clients) {

        client->stop();
        client->join();
    }

    clients.clear();
}


void Acceptor::stop() {

    _keep_running = false;
    try {

        acceptor_socket.shutdown(2);
        acceptor_socket.close();
    } catch (const LibError&) {}

    clear();
}


Acceptor::~Acceptor() {

    try {

        Acceptor::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << ACCEPTOR << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
