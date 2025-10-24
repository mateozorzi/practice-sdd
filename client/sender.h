#ifndef SENDER_H
#define SENDER_H
#include "common/queue.h"

#include "client_protocol.h"
#include "common/thread.h"

struct Event {
    // Forward Declaration
};

class Sender : public Thread {
    ClientProtocol& protocol;
    Queue<Event> events_queue;
    std::mutex queue_mutex;

public:
    explicit Sender(ClientProtocol& protocol);
    void add_event(Event event);
    void run() override;
};

#endif //SENDER_H