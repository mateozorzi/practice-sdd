#include "receiver.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#include "../common/liberror.h"
#include "../common/movement_dto.h"


Receiver::Receiver(ServerProtocol& server_protocol, Lobby& lobby, const uint32_t client_id,
                   Queue<GameCommand>& command_queue, const ServerConfig& config):
        server_protocol(server_protocol),
        lobby(lobby),
        client_id(client_id),
        command_queue(command_queue),
        config(config) {}

void Receiver::handle_get_car_catalogue() {

    std::vector<CarDTO> cars = {
        {"Compact", 
         static_cast<uint16_t>(config.app.client.cars.compact.health), 
         static_cast<uint8_t>(config.app.client.cars.compact.handling), 
         static_cast<uint8_t>(config.app.client.cars.compact.speed), 
         static_cast<uint8_t>(config.app.client.cars.compact.acceleration)},
        {"Sport", 
         static_cast<uint16_t>(config.app.client.cars.sport.health), 
         static_cast<uint8_t>(config.app.client.cars.sport.handling), 
         static_cast<uint8_t>(config.app.client.cars.sport.speed), 
         static_cast<uint8_t>(config.app.client.cars.sport.acceleration)},
        {"Convertible", 
         static_cast<uint16_t>(config.app.client.cars.convertible.health), 
         static_cast<uint8_t>(config.app.client.cars.convertible.handling), 
         static_cast<uint8_t>(config.app.client.cars.convertible.speed), 
         static_cast<uint8_t>(config.app.client.cars.convertible.acceleration)},
        {"Sedan", 
         static_cast<uint16_t>(config.app.client.cars.sedan.health), 
         static_cast<uint8_t>(config.app.client.cars.sedan.handling), 
         static_cast<uint8_t>(config.app.client.cars.sedan.speed), 
         static_cast<uint8_t>(config.app.client.cars.sedan.acceleration)},
        {"Jeep", 
         static_cast<uint16_t>(config.app.client.cars.jeep.health), 
         static_cast<uint8_t>(config.app.client.cars.jeep.handling), 
         static_cast<uint8_t>(config.app.client.cars.jeep.speed), 
         static_cast<uint8_t>(config.app.client.cars.jeep.acceleration)},
        {"Rally", 
         static_cast<uint16_t>(config.app.client.cars.rally.health), 
         static_cast<uint8_t>(config.app.client.cars.rally.handling), 
         static_cast<uint8_t>(config.app.client.cars.rally.speed), 
         static_cast<uint8_t>(config.app.client.cars.rally.acceleration)},
        {"Truck", 
         static_cast<uint16_t>(config.app.client.cars.truck.health), 
         static_cast<uint8_t>(config.app.client.cars.truck.handling), 
         static_cast<uint8_t>(config.app.client.cars.truck.speed), 
         static_cast<uint8_t>(config.app.client.cars.truck.acceleration)}
    };

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

        std::cerr << "[RECEIVER] Client " << client_id
            << " disconnected: " << e.what() << std::endl;

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
