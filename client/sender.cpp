#include "sender.h"

#include <chrono>
Sender::Sender(ClientProtocol& protocol): protocol(protocol) {}

void Sender::run() {
    while (should_keep_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(
                1));  // Solo de prueba, hay que enviar eventos del servidor
    }
}

void Sender::add_event(const Event event) { events_queue.try_push(event); }
