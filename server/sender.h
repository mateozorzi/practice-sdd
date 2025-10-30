#ifndef SENDER_H
#define SENDER_H

#include "../common/constants.h"
#include "../common/queue.h"
#include "../common/thread.h"

#include "game_loop.h"
#include "server_protocol.h"


class Sender final: public Thread {


    ServerProtocol& server_protocol;
    GameLoop& game_loop;
    Queue<CarPositionDTO> client_message_queue;
    uint32_t client_id;


public:
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    explicit Sender(ServerProtocol& server_protocol, GameLoop& game_loop, uint32_t client_id);

    void run() override;
    void stop() override;

    ~Sender() override;
};


#endif
