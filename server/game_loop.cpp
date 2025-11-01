#include "game_loop.h"

#include <chrono>

#include "../common/constants.h"
#include <vector>
#include "world_utils.h"
#include "../common/constants.h"
#include "world_creation_exception.h"


GameLoop::GameLoop(GameMonitor& game_monitor):
        command_queue(500), game_monitor(game_monitor), world(b2Vec2(0.0f, 0.0f)) {

    game_monitor.register_command_queue(command_queue);

    command_handlers = {
            {ClientRequest::SEND_USERNAME,
             [this](const GameCommand& game_command) { handle_player_username(game_command); }},
            {ClientRequest::MOVE_CAR,
             [this](const GameCommand& game_command) { handle_car_movement(game_command); }},
            {ClientRequest::DISCONNECT,
             [this](const GameCommand& game_command) { handle_disconnection(game_command); }}};
    
    auto polygons = WorldUtils::load_polygons_from_json(POLYGONS_LIBERTY_CITY.data());
    for (const auto& p: polygons) {
        try {
            bool added = WorldUtils::add_polygon_to_world(p, world);
            if (!added) {
                std::cout << "Polygon of size " << p.size() << " ommited\n";
            }
        } catch (const std::exception& e) {
            throw WorldCreationException(std::string("Could not add collision bodies: ") + std::string(e.what()));
        }
    }
}


void GameLoop::remove_car(const uint32_t client_id) {

    const auto it = cars.find(client_id);

    if (it != cars.end())
        cars.erase(it);
}


void GameLoop::process_commands() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) handle_command(game_command);
}


void GameLoop::handle_player_username(const GameCommand& game_command) {

    if (auto player = game_command.get_data<PlayerDTO>())
    
        std::cout << "New player with ID:" << game_command.get_client_id() << " has joined the game! | Username: " << player->get().username << std::endl;
}


void GameLoop::handle_car_movement(const GameCommand& game_command) {

    if (auto movement = game_command.get_data<MovementDTO>()) {

        auto [it, inserted] = cars.try_emplace(game_command.get_client_id(), world, CarConfig());
        it->second.handle_movement(movement->get());
    }
}

void GameLoop::handle_disconnection(const GameCommand& game_command) {

    remove_car(game_command.get_client_id());
}

void GameLoop::handle_command(const GameCommand& game_command) {

    const auto it = command_handlers.find(game_command.get_client_request());

    if (it != command_handlers.end())
        it->second(game_command);
}

void GameLoop::update_positions() {

    for (auto& [client_id, car]: cars) {

        car.update_position();
        game_monitor.broadcast(car.get_position_dto());
    }
}


void GameLoop::run() {

    try {

        while (should_keep_running()) {

            GameCommand game_command;
            try {

                process_commands();
            } catch (const ClosedQueue&) {

                break;
            }

            update_positions();
            world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    } catch (const ClosedQueue&) {}
}


void GameLoop::stop() {

    if (_keep_running)
        _keep_running = false;

    try {

        command_queue.close();
        cars.clear();
    } catch (const std::exception&) {}
}


GameLoop::~GameLoop() {

    try {

        GameLoop::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << GAME_LOOP << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
