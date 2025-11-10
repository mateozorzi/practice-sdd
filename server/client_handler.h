#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H


#include <string_view>

#include "../common/constants.h"
#include "../common/socket.h"

#include "receiver.h"
#include "sender.h"


class ClientHandler {


    Socket peer_socket;
    ServerProtocol server_protocol;
    GameMonitor& game_monitor;
    //Lobby& lobby;
    const uint32_t client_id;
    bool is_in_lobby = true;

    Receiver receiver;
    Sender sender;

    //void run_lobby_loop();
    //void extit_lobby();

    //void create_new_match();
    //void join_existing_match();

    void stop();


public:
    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

    explicit ClientHandler(Socket&& peer_socket, GameMonitor& game_monitor, uint32_t client_id);
    //explicit ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id);


    bool is_alive() const;

    ~ClientHandler();
};


#endif
