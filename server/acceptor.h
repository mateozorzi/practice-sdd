#ifndef ACCEPTOR_H
#define ACCEPTOR_H


#include <string>

#include "common/socket.h"
#include "common/thread.h"


constexpr std::string_view ACCEPTOR = "ACCEPTOR";
constexpr std::string_view CLIENT_CONNECTED = "A client has connected successfully!";
constexpr std::string_view EXCEPTION = "Exception";
constexpr std::string_view EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";


class Acceptor final: public Thread {


    Socket acceptor_socket;
    // std::list<std::unique_ptr<ClientHandler>> clients; (Only single client for now)

    // void reap(); (Only single client for now)


public:
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    explicit Acceptor(const std::string& port);

    void run() override;
    void stop() override;

    ~Acceptor() override;
};


#endif
