#include "client.h"

#include <any>
#include <memory>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/car_position_dto.h"
#include "../common/movement_dto.h"
#include "../common/player_dto.h"

#include "camera.h"
#include "menu_interface.h"

Client::Client(const std::string& hostname, const std::string& servname):
        client_socket(hostname.c_str(), servname.c_str()),
        sdl_manager(nullptr),
        protocol(client_socket),
        receiver(protocol),
        input_manager(protocol),
        menu(protocol),
        client_id(0) {}

void Client::run() {
    // lobby ui
    PlayerDTO player = menu.lobby();
    if (static_cast<int>(player.id) == -1) {
        exit(0);
    }

    client_id = player.id;

    // Recibir client_id del servidor antes de iniciar receiver
    // protocol.send_username("a");
    // client_id = protocol.receive_id();
    receiver.start();

    // Vida del jugador (0-100)
    int health = 100;  // Para probar como baja la barra de vida

    // ID - Posicion
    std::map<uint32_t, CarPositionDTO> cars;


    // sld_manager its initialized here
    std::string chosen_map = "liberty_city"; // TODO: remove harcoded
    sdl_manager = std::make_unique<SDLManager>();
    sdl_manager->load_bridges_rects(protocol.get_json("server/resources/world/bridges_" + chosen_map + ".json"));
    sdl_manager->load_decorations_rects(protocol.get_json("server/resources/world/decorations_" + chosen_map + ".json"));

    while (true) {
        if (!input_manager.handle_movement()) {
            return;
        }
        // Obtener posiciones
        Queue<std::vector<CarPositionDTO>>& car_queue = receiver.get_car_positions_queue();
        std::vector<CarPositionDTO> car_positions;
        bool has_positions = false;
        while (car_queue.try_pop(car_positions)) {
            has_positions = true;
        }
        if (has_positions) {
            std::vector<uint32_t> received_ids;
            for (CarPositionDTO& car_position: car_positions) {
                received_ids.push_back(car_position.client_id);
                cars[car_position.client_id] = car_position;
            }
            for (auto it = cars.begin(); it != cars.end();) {
                const bool found = std::ranges::any_of(
                        received_ids, [&](uint32_t id) { return id == it->first; });
                if (!found) {
                    it = cars.erase(it);
                } else {
                    ++it;
                }
            }
        }
        sdl_manager->render(player, health, cars, input_manager.is_moving());
    }
}

Client::~Client() {
    if (receiver.is_alive()) {
        receiver.stop();
        receiver.join();
    }
}
