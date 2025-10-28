#include "acceptor.h"

#include <iostream>

#include "common/liberror.h"


Acceptor::Acceptor(const std::string& port, GameLoop& game_loop): acceptor_socket(port.c_str()), game_loop(game_loop) {}


void Acceptor::run() {

    while (should_keep_running()) {

        try {

            Socket client_socket = acceptor_socket.accept();
            auto client = std::make_unique<ClientHandler>(std::move(client_socket), game_loop);
            clients.push_back(std::move(client));
            std::cout << CLIENT_CONNECTED << std::endl;
            reap();
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

            std::cout << "Reaping dead client..." << std::endl;
            return true;
        }

        return false;
    });

    std::cout << "Clients connected now: " << clients.size() << std::endl;
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
