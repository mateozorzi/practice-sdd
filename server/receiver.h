#ifndef RECEIVER_H
#define RECEIVER_H


#include "../common/constants.h"
#include "../common/thread.h"

//#include "game_loop.h"
#include "lobby.h"
#include "server_protocol.h"


class Receiver final: public Thread {


    ServerProtocol& server_protocol;
    Lobby& lobby;
    uint32_t client_id;
    Queue<GameCommand>& command_queue;

    void handle_player_username(const ClientRequest& client_request);
    void handle_car_selection();
    void handle_get_car_catalogue();
    void handle_car_movement(const ClientRequest& client_request);


public:
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    explicit Receiver(ServerProtocol& server_protocol, Lobby& lobby, 
                      uint32_t client_id, Queue<GameCommand>& command_queue);

    void run() override;
    void stop() override;

    ~Receiver() override;
};


#endif
