#ifndef SENDER_H
#define SENDER_H


#include "common/thread.h"


constexpr std::string_view SENDER = "SENDER";
constexpr std::string_view SENDER_EXCEPTION = "Exception";
constexpr std::string_view SENDER_EXCEPTION_IN_DESTRUCTOR = "Exception in destructor";

constexpr std::string_view SENDER_ACTIVATED = "Sender is now running!";


class Sender final: public Thread {


    uint32_t client_id;


public:
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    explicit Sender(uint32_t client_id);

    void run() override;
    void stop() override;

    ~Sender() override;
};


#endif
