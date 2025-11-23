#include "client.h"

#include <any>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/car_position_dto.h"
#include "../common/game_update_dto.h"
#include "../common/player_dto.h"

#include "menu_interface.h"

Client::Client(const std::string& hostname, const std::string& servname):
        client_socket(hostname.c_str(), servname.c_str()),
        sdl_manager(nullptr),
        protocol(client_socket),
        receiver(protocol),
        input_manager(*sdl_manager, protocol),
        menu(protocol),
        client_id(0) {}

void Client::run() {
    // lobby ui a
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
    // std::map<uint32_t, CarPositionDTO> cars;


    // sld_manager its initialized here
    sdl_manager = std::make_unique<SDLManager>();

    std::optional<WorldSnapshotDTO> last_world_snapshot;
    std::optional<RaceDTO> last_race_state;

    while (true) {
        if (!input_manager.handle_movement()) {
            return;
        }
        // Obtener posiciones
        Queue<GameUpdateDTO>& car_queue = receiver.get_car_positions_queue();

        bool has_positions = false;
        std::vector<GameUpdateDTO> snapshots;
        GameUpdateDTO snapshot;

        while (car_queue.try_pop(snapshot)) {
            has_positions = true;
            snapshots.push_back(snapshot);
        }
        if (has_positions) {
            for (auto& game_update: snapshots) {
                if (game_update.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {
                    last_world_snapshot = std::get<WorldSnapshotDTO>(game_update.data);
                } else if (game_update.gameloop_action == GameLoopAction::SEND_RACE) {
                    last_race_state = std::get<RaceDTO>(game_update.data);
                    sdl_manager->load_city(last_race_state.value().city);
                } else /*(std::holds_alternative<std::monostate>(snapshot.data))*/ {
                    // race finish by timeout
                }
            }
        }

        std::vector<std::pair<PlayerDTO, CarPositionDTO>> car_positions;

        if (last_world_snapshot.has_value()) {
            car_positions = last_world_snapshot.value().get_car_positions();
        }
        // std::cout << "[Client] next checkpoint id for player " << player.id << ": " <<
        // static_cast<int>(car_positions[0].second.next.id) << std::endl;

        sdl_manager->render(player, health, car_positions, input_manager.is_moving());
    }
}

Client::~Client() {
    if (receiver.is_alive()) {
        receiver.stop();
        receiver.join();
    }
}
