#include "receiver.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#include "../common/liberror.h"
#include "../common/movement_dto.h"


Receiver::Receiver(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t client_id,
                   Queue<GameCommand>& command_queue, const ServerConfig& server_config):
        server_protocol(server_protocol),
        lobby(lobby),
        client_id(client_id),
        command_queue(command_queue),
        server_config(server_config) {}

void Receiver::handle_get_car_catalogue() {

    std::vector<CarDTO> cars = {

            {"Compact", static_cast<uint16_t>(server_config.physics.compact.health),
             static_cast<uint8_t>((server_config.physics.compact.turn_torque + server_config.physics.compact.drift_factor) * 100),
             static_cast<uint8_t>(server_config.physics.compact.max_speed),
             static_cast<uint8_t>(server_config.physics.compact.acceleration)},

            {"Sport", static_cast<uint16_t>(server_config.physics.sport.health),
             static_cast<uint8_t>((server_config.physics.sport.turn_torque + server_config.physics.sport.drift_factor) * 100),
             static_cast<uint8_t>(server_config.physics.sport.max_speed),
             static_cast<uint8_t>(server_config.physics.sport.acceleration)},

            {"Convertible", static_cast<uint16_t>(server_config.physics.convertible.health),
             static_cast<uint8_t>((server_config.physics.convertible.turn_torque + server_config.physics.convertible.drift_factor) * 100),
             static_cast<uint8_t>(server_config.physics.convertible.max_speed),
             static_cast<uint8_t>(server_config.physics.convertible.acceleration)},
    
            {"Sedan", static_cast<uint16_t>(server_config.physics.sedan.health),
             static_cast<uint8_t>((server_config.physics.sedan.turn_torque + server_config.physics.sedan.drift_factor) * 100),
             static_cast<uint8_t>(server_config.physics.sedan.max_speed),
             static_cast<uint8_t>(server_config.physics.sedan.acceleration)},
    
            {"Jeep", static_cast<uint16_t>(server_config.physics.jeep.health),
             static_cast<uint8_t>((server_config.physics.jeep.turn_torque + server_config.physics.jeep.drift_factor) * 100),
             static_cast<uint8_t>(server_config.physics.jeep.max_speed),
             static_cast<uint8_t>(server_config.physics.jeep.acceleration)},

            {"Rally", static_cast<uint16_t>(server_config.physics.rally.health),
             static_cast<uint8_t>((server_config.physics.rally.turn_torque + server_config.physics.rally.drift_factor) * 100),
             static_cast<uint8_t>(server_config.physics.rally.max_speed),
             static_cast<uint8_t>(server_config.physics.rally.acceleration)},

            {"Truck", static_cast<uint16_t>(server_config.physics.truck.health),
             static_cast<uint8_t>((server_config.physics.truck.turn_torque + server_config.physics.truck.drift_factor) * 100),
             static_cast<uint8_t>(server_config.physics.truck.max_speed),
             static_cast<uint8_t>(server_config.physics.truck.acceleration)}};

    CarCatalogueDTO catalogue(std::move(cars));

    server_protocol.send_car_catalogue(catalogue);
}

void Receiver::handle_get_json_file(const std::string& file_path) {
    server_protocol.send_json_file(file_path);
}


void Receiver::run() {

    try {

        while (should_keep_running()) {

            GameCommandDTO game_command = server_protocol.receive_game_command();
            if (game_command.client_action == ClientAction::GET_CAR_CATALOGUE) {

                handle_get_car_catalogue();
                continue;
            } else if (game_command.client_action == ClientAction::GET_JSON_FILE) {

                auto file_path_opt = game_command.get_data<std::string>();
                if (file_path_opt.has_value()) {
                    handle_get_json_file(file_path_opt.value().get());
                }
                continue;
            } else if (game_command.client_action == ClientAction::DISCONNECT) {

                command_queue.push(GameCommand(client_id, std::move(game_command)));
                break;
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
