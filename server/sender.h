#ifndef SENDER_H
#define SENDER_H


#include "../common/constants.h"
#include "../common/thread.h"

#include "lobby.h"
#include "server_protocol.h"


constexpr std::string_view SENDER = "SENDER";


class Sender final {


    ServerProtocol& server_protocol;
    Lobby& lobby;
    uint32_t player_id;
    Queue<GameUpdateDTO>& game_update_queue;
    std::atomic_bool keep_running = true;

    void sender_loop();


public:
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    explicit Sender(ServerProtocol& server_protocol, Lobby& lobby, uint32_t player_id,
                    Queue<GameUpdateDTO>& game_update_queue);

    void run();
    void stop();

    ~Sender();
};


#endif
