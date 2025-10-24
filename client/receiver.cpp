#include "receiver.h"
#include <chrono>

Receiver::Receiver(ClientProtocol &protocol) : protocol(protocol) {}

GameState Receiver::get_game_state() const {
    return game_state;
}

void Receiver::run() {
    while (should_keep_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Solo de prueba, hay que recibir eventos del servidor
    }
}
