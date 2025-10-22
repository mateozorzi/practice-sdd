#include "client_handler.h"

#include <iostream>
#include <utility>

#include "common/liberror.h"


ClientHandler::ClientHandler(Socket&& socket):
        client_socket(std::move(socket)),
        client_id(1) {} // Should construct a sender and receiver thread


bool ClientHandler::is_alive() const { return true; } // Shoukd change this later!


void ClientHandler::stop() {

    // This method does nothing for now, it should have a sender and receiver thread later
}

ClientHandler::~ClientHandler() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
