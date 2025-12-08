#include "receiver.h"

#include <utility>
#include <vector>

#include "../common/liberror.h"


Receiver::Receiver(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t player_id,
                   Queue<GameCommand>& command_queue):
        server_protocol(server_protocol),
        lobby(lobby),
        player_id(player_id),
        command_queue(command_queue) {}


void Receiver::run() {

    try {

        while (should_keep_running()) {

            GameCommandDTO game_command = server_protocol.receive_game_command();

            command_queue.push(GameCommand(player_id, std::move(game_command)));

            if (game_command.client_action == ClientAction::DISCONNECT) {

                _is_alive = false;
                break;
            }
        }
    } catch (const ClosedQueue&) {

    } catch (const LibError& e) {

    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << EXCEPTION << ": " << e.what();
    }
}


void Receiver::stop() {

    if (_keep_running)
        _keep_running = false;
}


Receiver::~Receiver() {

    std::cerr << "[" << RECEIVER << "] Stopping." << std::endl;
    try {

        Receiver::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
