#include "client_handler.h"

#include <iostream>
#include <utility>

#include "common/liberror.h"


ClientHandler::ClientHandler(Socket&& socket):
        client_socket(std::move(socket)),
        client_id(1),
        receiver(client_id),
        sender(client_id) {

    receiver.start();
    sender.start();
}


bool ClientHandler::is_alive() const { return receiver.is_alive() || sender.is_alive(); } // Shoukd change this later!


void ClientHandler::stop() {

    receiver.stop();
    sender.stop();

    receiver.join();
    sender.join();
}

ClientHandler::~ClientHandler() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
