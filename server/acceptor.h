#ifndef ACCEPTOR_H
#define ACCEPTOR_H


#include <list>
#include <memory>
#include <string>

#include "../common/constants.h"
#include "../common/socket.h"
#include "../common/thread.h"

#include "client_handler.h"


class Acceptor final: public Thread {


    Socket acceptor_socket;
    std::list<std::unique_ptr<ClientHandler>> clients;
    GameMonitor& game_monitor;
    uint32_t client_id = 0;

    void reap();


public:
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    explicit Acceptor(const std::string& port, GameMonitor& game_monitor);

    void run() override;
    void stop() override;

    ~Acceptor() override;
};


#endif
