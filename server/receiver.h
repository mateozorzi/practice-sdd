#ifndef RECEIVER_H
#define RECEIVER_H


#include "../common/constants.h"
#include "../common/thread.h"

#include "lobby.h"
#include "server_protocol.h"


constexpr std::string_view RECEIVER = "RECEIVER";


class Receiver final: public Thread {


    ServerProtocol& server_protocol;
    Lobby& lobby;
    uint32_t player_id;
    Queue<GameCommand>& command_queue;


public:
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    explicit Receiver(ServerProtocol& server_protocol, Lobby& lobby, uint32_t player_id,
                      Queue<GameCommand>& command_queue);

    void run() override;
    void stop() override;

    ~Receiver() override;
};


#endif
