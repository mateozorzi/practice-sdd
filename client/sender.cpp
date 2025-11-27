#include "sender.h"

#include "common/client_event_dto.h"

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

void Sender::add_event(const ClientEventDTO& event) { events_queue.try_push(event); }

void Sender::send_event() {
    const ClientEventDTO event = events_queue.pop();
    //print if its clientactiondto
    if (std::holds_alternative<ClientActionDTO>(event)) {
        const ClientActionDTO& action = std::get<ClientActionDTO>(event);
        if (action.action == ClientAction::DISCONNECT) {
            std::cout << "[SENDER] Sending DISCONNECT action to server..." << std::endl;
        }
    }
    protocol.send_event(event);
    std::cout << "[SENDER] Event sent to server." << std::endl;
}

void Sender::close_queue() {
    try {
        events_queue.close();
    } catch (const std::exception& e) {}
}
