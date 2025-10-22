#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H


#include "common/socket.h"

#include <cstdint>
#include <string_view>


constexpr std::string_view CLIENT_HANDLER = "CLIENT HANDLER";
constexpr std::string_view EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";


class ClientHandler {


    Socket client_socket;
    uint32_t client_id;

    void stop();


public:
    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

    explicit ClientHandler(Socket&& socket);

    bool is_alive() const;

    ~ClientHandler();
};


#endif
