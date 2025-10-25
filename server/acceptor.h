#ifndef ACCEPTOR_H
#define ACCEPTOR_H


#include <list>
#include <memory>
#include <string>

#include "common/socket.h"
#include "common/thread.h"

#include "client_handler.h"


constexpr std::string_view ACCEPTOR = "ACCEPTOR";
constexpr std::string_view CLIENT_CONNECTED = "A client has connected successfully!";
constexpr std::string_view EXCEPTION = "Exception";


class Acceptor final: public Thread {


    Socket acceptor_socket;
    std::list<std::unique_ptr<ClientHandler>> clients;
    GameLoop& game_loop;

    void reap();


public:
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    explicit Acceptor(const std::string& port, GameLoop& game_loop);

    void run() override;
    void stop() override;

    ~Acceptor() override;
};


#endif
