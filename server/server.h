#ifndef SERVER_H
#define SERVER_H


#include <string>

#include "acceptor.h"
#include "game_loop.h"
#include "lobby.h"
#include "server_config.h"
#include "server_constants.h"


class Server {

    const ServerConfig& server_config;
    Lobby lobby;
    Acceptor clients_acceptor;

    void stop();


public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(const std::string& port, const ServerConfig& config);

    void run();

    ~Server();
};


#endif
