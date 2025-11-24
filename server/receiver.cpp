#include "receiver.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#include "../common/liberror.h"
#include "../common/movement_dto.h"


Receiver::Receiver(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t client_id,
                   Queue<GameCommand>& command_queue):
        server_protocol(server_protocol),
        lobby(lobby),
        client_id(client_id),
        command_queue(command_queue) {}

void Receiver::handle_get_json_file(const std::string& file_path) {
    server_protocol.send_json_file(file_path);
}


void Receiver::run() {

    try {

        while (should_keep_running()) {

            GameCommandDTO game_command = server_protocol.receive_game_command();
            if (game_command.client_action == ClientAction::GET_JSON_FILE) {

                auto file_path_opt = game_command.get_data<std::string>();
                if (file_path_opt.has_value())
                    handle_get_json_file(file_path_opt.value().get());
                continue;
            }

            command_queue.push(GameCommand(client_id, std::move(game_command)));
        }
    } catch (const LibError& e) {

    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << EXCEPTION << ": " << e.what();
    }
}


void Receiver::stop() {

    if (_keep_running) {

        _keep_running = false;
    }
}

Receiver::~Receiver() {

    try {

        Receiver::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
