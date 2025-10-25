#ifndef SENDER_H
#define SENDER_H

#include "common/queue.h"
#include "common/thread.h"

#include "game_loop.h"
#include "server_protocol.h"


constexpr std::string_view SENDER = "SENDER";
constexpr std::string_view SENDER_EXCEPTION = "Exception";
constexpr std::string_view SENDER_EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";


class Sender final: public Thread {


    ServerProtocol& server_protocol;
    Queue<CarPositionDTO> client_message_queue;
    uint32_t client_id;


public:
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    explicit Sender(ServerProtocol& server_protocol, uint32_t client_id);

    void run() override;
    void stop() override;

    ~Sender() override;
};


#endif
