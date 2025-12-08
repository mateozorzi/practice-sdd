#include "sender.h"

#include "../common/game_update_dto.h"
#include "../common/liberror.h"


Sender::Sender(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t player_id,
               Queue<GameUpdateDTO>& game_update_queue):
        server_protocol(server_protocol),
        lobby(lobby),
        player_id(player_id),
        game_update_queue(game_update_queue) {}


void Sender::sender_loop() {

    while (keep_running) {

        try {

            GameUpdateDTO game_update = game_update_queue.pop();

            server_protocol.send_game_update(game_update);

        } catch (const ClosedQueue&) {

            keep_running = false;
            break;
        }
    }
}


void Sender::run() {

    try {

        sender_loop();
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << EXCEPTION << ": " << e.what();
    }
}


void Sender::stop() {

    if (keep_running)
        keep_running = false;
}


Sender::~Sender() {

    std::cerr << "[" << SENDER << "] Stopping." << std::endl;
    try {

        Sender::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
