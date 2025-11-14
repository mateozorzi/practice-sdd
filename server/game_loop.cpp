#include "game_loop.h"

#include <chrono>
#include <string>
#include <vector>

#include "../common/constants.h"

#include "world_creation_exception.h"
#include "world_utils.h"


GameLoop::GameLoop(GameMonitor& game_monitor, Queue<GameCommand>& command_queue,
                   const uint8_t max_players, const ServerConfig& config):
        max_players(max_players),
        config(config),
        game_monitor(game_monitor),
        command_queue(command_queue),
        world(b2Vec2(0.0f, 0.0f)),
        collision_listener(players) {

    world.SetContactListener(&collision_listener);

    command_handlers = {
            {ClientAction::SEND_CAR_SELECTION,
             [this](const GameCommand& game_command) { handle_car_selection(game_command); }},
            {ClientAction::SEND_CAR_MOVEMENT,
             [this](const GameCommand& game_command) { handle_car_movement(game_command); }},
            {ClientAction::DISCONNECT,
             [this](const GameCommand& game_command) { handle_disconnection(game_command); }}};

    auto polygons = WorldUtils::load_polygons_from_json(POLYGONS_LIBERTY_CITY.data());
    for (const auto& p: polygons) {
        try {
            bool added = WorldUtils::add_polygon_to_world(p, world);
            if (!added) {
                std::cout << "Polygon of size " << p.size() << " ommited\n";
            }
        } catch (const std::exception& e) {
            throw WorldCreationException(std::string("Could not add collision bodies: ") +
                                         std::string(e.what()));
        }
    }
}


void GameLoop::remove_player(const uint32_t client_id) {

    const auto it = players.find(client_id);

    if (it != players.end())
        players.erase(it);
    
    game_monitor.unregister_client(client_id);
    game_monitor.remove_client(client_id);
}

bool GameLoop::is_full() const { return players.size() >= max_players; }

bool GameLoop::is_empty() const { return players.empty(); }


void GameLoop::add_player(const uint32_t client_id, const std::string& username) {

    players.try_emplace(client_id, client_id, username, world, config);

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << client_id
              << " has joined the game. | Username: " << username << std::endl
              << std::endl;
}

void GameLoop::process_commands() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) handle_command(game_command);
}


void GameLoop::handle_car_selection(const GameCommand& game_command) {

    if (auto car = game_command.get_data<car_dto>()) {
        uint32_t client_id = game_command.get_client_id();
        car_dto car_config = car->get();
        auto it = players.find(client_id);
        if (it != players.end()) {
            it->second.set_car_configuration(car_config);
            std::string model = it->second.get_car().model;
            std::cout << "Player with ID: " << it->second.get_client_id()
                      << " has car model set to: " << model << std::endl;
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

    std::cerr << "[" << GAME_LOOP << "] "
            << "Player with ID: " << game_command.get_client_id() << " has disconnected." << std::endl;

    remove_player(game_command.get_client_id());
}

void GameLoop::handle_command(const GameCommand& game_command) {

    const auto it = command_handlers.find(game_command.get_client_action());

    if (it != command_handlers.end())
        it->second(game_command);
}

void GameLoop::update_positions() {

    WorldSnapshotDTO world_snapshot;

    for (auto& [client_id, player]: players) {

        player.update_car_position();
        car_dto car = player.get_car();
        PlayerDTO player_dto(player.get_username(), player.get_client_id(), player.get_car());
        world_snapshot.add_car_position(player_dto, player.get_car_position());
    }

    game_monitor.broadcast(world_snapshot);
}


bool GameLoop::all_player_in_game() const {

    return std::all_of(players.begin(), players.end(),
                       [](const auto& p) { return p.second.is_in_game(); });
}


void GameLoop::run() {

    try {

        const int game_frame_ms = static_cast<int>(1000.0f / config.app.game.game_fps);
        const int waiting_frame_ms = static_cast<int>(1000.0f / config.app.server.waiting_for_players_fps);

        while (should_keep_running()) {

            process_commands();

            if (all_player_in_game()) {

                update_positions();
                world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
                std::this_thread::sleep_for(std::chrono::milliseconds(game_frame_ms));
            } else {

                std::this_thread::sleep_for(std::chrono::milliseconds(waiting_frame_ms));
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
