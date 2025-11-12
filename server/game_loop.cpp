#include "game_loop.h"

#include <chrono>

#include "../common/constants.h"
#include <vector>
#include "world_utils.h"
#include "../common/constants.h"
#include "world_creation_exception.h"


GameLoop::GameLoop(GameMonitor& game_monitor, Queue<GameCommand>& command_queue):
        game_monitor(game_monitor), command_queue(command_queue), world(b2Vec2(0.0f, 0.0f)), 
        collision_listener(players) {
    
    world.SetContactListener(&collision_listener);

    command_handlers = {
            {ClientRequest::SEND_CAR_SELECTION,
             [this](const GameCommand& game_command) { handle_car_selection(game_command); }},
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


void GameLoop::remove_player(const uint32_t client_id) {

    const auto it = players.find(client_id);

    if (it != players.end())
        players.erase(it);
}

bool GameLoop::is_full() const {

    return players.size() == 1;
}

bool GameLoop::is_empty() const {

    return players.empty();
}


void GameLoop::add_player(const uint32_t client_id, const std::string& username) {

        players.try_emplace(
        client_id,
        client_id,
        username,
        world
    );

    std::cout << "New player with ID: " << client_id 
        << " has joined a game! | Username: " << username << std::endl << std::endl;
}

void GameLoop::process_commands() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) handle_command(game_command);
}


void GameLoop::handle_car_selection(const GameCommand& game_command) {

    if (auto car = game_command.get_data<car_dto>()) {

        uint32_t client_id = game_command.get_client_id();
        auto it = players.find(client_id);
        if (it != players.end()) {
            it->second.set_car_configuration(car->get());
            it->second.set_in_game();
        }
    }
}


void GameLoop::handle_car_movement(const GameCommand& game_command) {

    if (auto movement = game_command.get_data<MovementDTO>()) {

        uint32_t client_id = game_command.get_client_id();
        auto it = players.find(client_id);
        if (it != players.end())
            it->second.handle_car_movement(movement->get());
    }
}

void GameLoop::handle_disconnection(const GameCommand& game_command) {

    remove_player(game_command.get_client_id());
}

void GameLoop::handle_command(const GameCommand& game_command) {

    const auto it = command_handlers.find(game_command.get_client_request());

    if (it != command_handlers.end())
        it->second(game_command);
}

void GameLoop::update_positions() {

    WorldSnapshot world_snapshot;
    world_snapshot.reserve(players.size());

    for (auto& [client_id, player]: players) {

        player.update_car_position();
        world_snapshot.add_car_position(player.get_car_position());
    }

    game_monitor.broadcast(world_snapshot);
}


bool GameLoop::all_player_in_game() const {

    return std::all_of(players.begin(), players.end(),
                       [](const auto& p) { return p.second.is_in_game(); });
}


void GameLoop::run() {

    try {

        while (should_keep_running()) {

            process_commands();

            if (all_player_in_game()) {

                update_positions();
                world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            } else {

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    } catch (const ClosedQueue&) {}
}


void GameLoop::stop() {

    if (_keep_running)
        _keep_running = false;

    try {

        players.clear();
    } catch (const std::exception&) {}
}


GameLoop::~GameLoop() {

    try {

        GameLoop::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << GAME_LOOP << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
