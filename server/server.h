#ifndef SERVER_H
#define SERVER_H


#include <string>

#include "acceptor.h"
#include "game_loop.h"
#include "lobby.h"
#include "server_config.h"


constexpr std::string_view SERVER = "SERVER";
constexpr char SERVER_CMD_EXIT = 'q';
constexpr std::string_view SERVER_STARTED = "SERVER STARTED";
constexpr std::string_view SERVER_STOPPED = "SERVER STOPPED";
constexpr std::string_view SERVER_CONFIG_FILEPATH = "/etc/nfs";


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
