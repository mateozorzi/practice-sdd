#ifndef RECEIVER_H
#define RECEIVER_H


#include "../common/constants.h"
#include "../common/thread.h"

#include "game_loop.h"
#include "server_protocol.h"


class Receiver final: public Thread {


    ServerProtocol& server_protocol;
    GameMonitor& game_monitor;
    uint32_t client_id;

    void handle_car_movement(const ClientRequest& client_request);


public:
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    explicit Receiver(ServerProtocol& server_protocol, GameMonitor& game_monitor, uint32_t client_id);

    void run() override;
    void stop() override;

    ~Receiver() override;
};


#endif
