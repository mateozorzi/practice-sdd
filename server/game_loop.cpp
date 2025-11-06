#include "game_loop.h"

#include <chrono>

#include "../common/constants.h"
#include <vector>
#include "world_utils.h"
#include "../common/constants.h"
#include "world_creation_exception.h"


GameLoop::GameLoop(GameMonitor& game_monitor):
        command_queue(500), game_monitor(game_monitor), world(b2Vec2(0.0f, 0.0f)), 
        collision_listener(players) {

    game_monitor.register_command_queue(command_queue);
    
    world.SetContactListener(&collision_listener);

    command_handlers = {
            {ClientRequest::SEND_USERNAME,
             [this](const GameCommand& game_command) { handle_player_username(game_command); }},
            {ClientRequest::MOVE_CAR,
             [this](const GameCommand& game_command) { handle_car_movement(game_command); }},
            {ClientRequest::DISCONNECT,
             [this](const GameCommand& game_command) { handle_disconnection(game_command); }}};
    
    /*auto polygons = WorldUtils::load_polygons_from_json(POLYGONS_LIBERTY_CITY.data());
    for (const auto& p: polygons) {
        try {
            bool added = WorldUtils::add_polygon_to_world(p, world);
            if (!added) {
                std::cout << "Polygon of size " << p.size() << " ommited\n";
            }
        } catch (const std::exception& e) {
            throw WorldCreationException(std::string("Could not add collision bodies: ") + std::string(e.what()));
        }
    }*/

    // Hardcoding the first square of the city (just for testing collisions)

    struct BoxVertices {
        float min_x, min_y, max_x, max_y;
    };

    std::vector<BoxVertices> boxes = {
        {136.0f, 144.0f, 311.0f, 223.0f},
        {160.0f, 224.0f, 287.0f, 255.0f},
        {136.0f, 256.0f, 311.0f, 303.0f},
        {160.0f, 304.0f, 287.0f, 335.0f},
        {136.0f, 336.0f, 311.0f, 399.0f},
        {160.0f, 400.0f, 287.0f, 431.0f},
        {136.0f, 432.0f, 311.0f, 479.0f}
    };

    // float zoom = 3.5f;
    for (size_t i = 0; i < boxes.size(); ++i) {
        const auto& box = boxes[i];
        
        float min_x = box.min_x / PIXELS_PER_METER;
        float max_x = box.max_x / PIXELS_PER_METER;
        float min_y = box.min_y / PIXELS_PER_METER;
        float max_y = box.max_y / PIXELS_PER_METER;
        
        float center_x = (min_x + max_x) / 2.0f;
        float center_y = (min_y + max_y) / 2.0f;
        float half_width = (max_x - min_x) / 2.0f;
        float half_height = (max_y - min_y) / 2.0f;
        
        b2BodyDef body_def;
        body_def.position.Set(center_x, center_y);
        body_def.type = b2_staticBody;
        
        b2Body* box_body = world.CreateBody(&body_def);
        
        b2PolygonShape box_shape;
        box_shape.SetAsBox(half_width, half_height);
        
        b2FixtureDef fixture_def;
        fixture_def.shape = &box_shape;
        fixture_def.density = 0.0f;
        fixture_def.friction = 0.5f;
        
        box_body->CreateFixture(&fixture_def);
    }
}


void GameLoop::remove_car(const uint32_t client_id) {

    const auto it = players.find(client_id);

    if (it != players.end())
        players.erase(it);
}


void GameLoop::process_commands() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) handle_command(game_command);
}


void GameLoop::handle_player_username(const GameCommand& game_command) {

    if (auto player_dto = game_command.get_data<PlayerDTO>()) {

        uint32_t client_id = game_command.get_client_id();
        players.try_emplace(
            client_id, 
            client_id, 
            player_dto->get().username, 
            world, 
            CarConfig()
        );

        std::cout << "New player with ID: " << client_id 
                << " has joined the game! | Username: " 
                << player_dto->get().username << std::endl;
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

    remove_car(game_command.get_client_id());
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
