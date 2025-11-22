#ifndef SENDER_H
#define SENDER_H
#include "common/client_event_dto.h"
#include "common/queue.h"
#include "common/thread.h"

#include "client_protocol.h"

class Sender: public Thread {
    ClientProtocol& protocol;
    Queue<ClientEventDTO> events_queue;
    std::mutex queue_mutex;

public:
    explicit Sender(ClientProtocol& protocol);
    void add_event(const ClientEventDTO& event);
    void send_event();
    void close_queue();
    void run() override;
};

#endif  // SENDER_H
