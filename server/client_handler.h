#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H


#include <string_view>

#include "../common/constants.h"
#include "../common/socket.h"

#include "receiver.h"
#include "sender.h"


class ClientHandler {


    Socket client_socket;
    ServerProtocol server_protocol;
    GameLoop& game_loop;
    uint32_t client_id;

    Receiver receiver;
    Sender sender;

    void stop();


public:
    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

    explicit ClientHandler(Socket&& socket, GameLoop& game_loop, uint32_t client_id);

    bool is_alive() const;

    ~ClientHandler();
};


#endif
