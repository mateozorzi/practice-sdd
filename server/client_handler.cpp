#include "client_handler.h"

#include <iostream>
#include <utility>

#include "common/liberror.h"


ClientHandler::ClientHandler(Socket&& socket, GameMonitor& game_monitor, uint32_t client_id):
        client_socket(std::move(socket)),
        server_protocol(client_socket),
        game_monitor(game_monitor),
        client_id(client_id),
        receiver(server_protocol, game_monitor, client_id),
        sender(server_protocol, game_monitor, client_id) {

    receiver.start();
    sender.start();
}


bool ClientHandler::is_alive() const { return receiver.is_alive() && sender.is_alive(); }


void ClientHandler::stop() {

    try {

        server_protocol.stop();
    } catch (const LibError&) {}

    receiver.stop();
    sender.stop();

    receiver.join();
    sender.join();

    game_monitor.remove_client(client_id);
}

ClientHandler::~ClientHandler() {

    try {

        stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << CLIENT_HANDLER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
