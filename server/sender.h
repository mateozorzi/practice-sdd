#ifndef SENDER_H
#define SENDER_H

#include "../common/constants.h"
#include "../common/queue.h"
#include "../common/thread.h"

#include "lobby.h"
#include "server_protocol.h"


class Sender final {


    ServerProtocol& server_protocol;
    Lobby& lobby;
    uint32_t client_id;
    Queue<WorldSnapshotDTO>& snapshot_queue;
    std::atomic_bool keep_running;


public:
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    explicit Sender(ServerProtocol& server_protocol, Lobby& lobby, uint32_t client_id,
                    Queue<WorldSnapshotDTO>& snapshot_queue);

    void run();
    bool is_alive();
    void stop();

    ~Sender();
};


#endif
