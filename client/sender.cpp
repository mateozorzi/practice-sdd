#include "sender.h"

Sender::Sender(ClientProtocol& protocol): protocol(protocol) {}

void Sender::run() {
    while (should_keep_running()) {
        try {
            send_event();
        } catch (const std::exception& e) {
            break;
        }
    }
}

void Sender::add_event(const MovementDTO movement) { events_queue.try_push(movement); }

void Sender::send_event() {
    const MovementDTO movement = events_queue.pop();
    protocol.send_movement(movement);
}

void Sender::close_queue() {
    try {
        events_queue.close();
    } catch (const std::exception& e) {}
}
