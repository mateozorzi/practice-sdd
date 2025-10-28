#ifndef RECEIVER_H
#define RECEIVER_H


#include "common/thread.h"

#include "game_loop.h"
#include "server_protocol.h"


constexpr std::string_view RECEIVER = "RECEIVER";
constexpr std::string_view RECEIVER_EXCEPTION = "Exception";
constexpr std::string_view RECEIVER_EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";

constexpr std::string_view RECEIVER_ACTIVATED = "Receiver is now running!";


class Receiver final: public Thread {


    ServerProtocol& server_protocol;
    GameLoop& game_loop;
    uint32_t client_id;

    void handle_car_movement(const ClientRequest& client_request);


public:
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    explicit Receiver(ServerProtocol& server_protocol, GameLoop& game_loop, uint32_t client_id);

    void run() override;
    void stop() override;

    ~Receiver() override;
};


#endif
