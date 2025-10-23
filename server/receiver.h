#ifndef RECEIVER_H
#define RECEIVER_H


#include "common/thread.h"


constexpr std::string_view RECEIVER = "RECEIVER";
constexpr std::string_view RECEIVER_EXCEPTION = "Exception";
constexpr std::string_view RECEIVER_EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";

constexpr std::string_view RECEIVER_ACTIVATED = "Receiver is now running!";


class Receiver final: public Thread {


    uint32_t client_id;


public:
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    explicit Receiver(uint32_t client_id);

    void run() override;
    void stop() override;

    ~Receiver() override;
};


#endif
