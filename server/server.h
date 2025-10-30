#ifndef SERVER_H
#define SERVER_H


#include <string>

#include "../common/constants.h"

#include "acceptor.h"
#include "game_loop.h"


class Server {

    GameLoop game_loop;
    Acceptor clients_acceptor;

    void stop();


public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(const std::string& port);

    void run();

    ~Server();
};


#endif
