#include "client_handler.h"

#include <iostream>
#include <utility>

#include "common/liberror.h"


ClientHandler::ClientHandler(Socket&& socket):
        client_socket(std::move(socket)),
        client_id(1),
        sender(client_id) {

    sender.start();
} // Should construct a receiver thread


bool ClientHandler::is_alive() const { return sender.is_alive(); } // Shoukd change this later!


void ClientHandler::stop() {

    sender.stop();

    sender.join();
}

ClientHandler::~ClientHandler() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
