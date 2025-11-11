#ifndef SENDER_H
#define SENDER_H

#include "../common/constants.h"
#include "../common/queue.h"
#include "../common/thread.h"

//#include "game_monitor.h"
#include "lobby.h"
#include "server_protocol.h"


class Sender final: public Thread {


    ServerProtocol& server_protocol;
    //GameMonitor& game_monitor;
    Lobby& lobby;
    uint32_t client_id;
    Queue<WorldSnapshot>& snapshot_queue;


public:
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    explicit Sender(ServerProtocol& server_protocol, Lobby& lobby,
                    uint32_t client_id, Queue<WorldSnapshot>& snapshot_queue);

    void run() override;
    void stop() override;

    ~Sender() override;
};


#endif
