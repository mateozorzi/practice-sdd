#ifndef SERVER_H
#define SERVER_H


#include <string>

#include "acceptor.h"
#include "game_loop.h"
<<<<<<< HEAD
#include "server_config.h"
=======
>>>>>>> 6263648 (Feature: Second approach to multigame server.)
#include "lobby.h"


class Server {

    ServerConfig config;
    //Lobby lobby
    GameMonitor game_monitor;
    GameLoop game_loop;
    Acceptor clients_acceptor;

    void stop();


public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(const std::string& port, ServerConfig config);

    void run();

    ~Server();
};


#endif
