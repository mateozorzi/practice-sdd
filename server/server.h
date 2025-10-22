#ifndef SERVER_H
#define SERVER_H


#include "acceptor.h"

#include <string>


constexpr char SERVER_CMD_EXIT = 'q';

constexpr std::string_view SERVER_STARTED = "SERVER STARTED";
constexpr std::string_view SERVER_STOPPED = "SERVER STOPPED";

constexpr std::string_view SERVER = "SERVER";



class Server {

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
