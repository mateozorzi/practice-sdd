#include "game_loop.h"

#include <chrono>
#include <string>
#include <vector>

#include "../common/constants.h"

#include "world_creation_exception.h"
#include "world_utils.h"
#include "../common/checkpoint_dto.h"
#include "../common/leaderboard_dto.h"


GameLoop::GameLoop(GameMonitor& game_monitor, Queue<GameCommand>& command_queue,
                   const uint8_t max_players, const ServerConfig& config, RaceCatalogueDTO& selected) :
        max_players(max_players),
        config(config),
        game_monitor(game_monitor),
        command_queue(command_queue),
        world(b2Vec2(0.0f, 0.0f)),
        collision_listener(players),
        selected_races(selected),
        leaderboards(), actual_race(), final_leaderboard(),
        race_active(false) {

    world.SetContactListener(&collision_listener);

    command_handlers = {
            {ClientAction::SEND_CAR_SELECTION,
             [this](const GameCommand& game_command) { handle_car_selection(game_command); }},
            {ClientAction::SEND_CAR_MOVEMENT,
             [this](const GameCommand& game_command) { handle_car_movement(game_command); }},
            {ClientAction::DISCONNECT,
             [this](const GameCommand& game_command) { handle_disconnection(game_command); }},
            {ClientAction::FINISH_RACE,
             [this](const GameCommand& game_command) { handle_finish_race(game_command); }}
    };


    auto on_ground_collidables = WorldUtils::load_polygons_from_json(POLYGONS_LIBERTY_CITY.data());
    for (const auto& ground_collidable: on_ground_collidables) {
        try {
            bool added = WorldUtils::add_polygon_to_world(ground_collidable, world, CATEGORY_GROUND, MASK_GROUND);
            if (!added) {
                std::cout << "On ground collidable of size " << ground_collidable.size() << " ommited\n";
            }
        } catch (const std::exception& e) {
            throw WorldCreationException(std::string("Could not add on ground collidable bodies: ") +
                                         std::string(e.what()));
        }
    }

    auto on_bridge_collidables = WorldUtils::load_polygons_from_json("server/resources/world/on_bridge_collidables_liberty_city.json");
    for (const auto& bridge_collidable: on_bridge_collidables) {
        try {
            bool added = WorldUtils::add_polygon_to_world(bridge_collidable, world, CATEGORY_BRIDGE, MASK_BRIDGE);
            if (!added) {
                std::cout << "On bridge collidable of size " << bridge_collidable.size() << " ommited\n";
            }
        } catch (const std::exception& e) {
            throw WorldCreationException(std::string("Could not add on bridge collidable bodies: ") +
                                         std::string(e.what()));
        }
    }

    auto on_bridge_sensors = WorldUtils::load_polygons_from_json("server/resources/world/on_bridge_sensors_liberty_city.json");
    for (const auto& p: on_bridge_sensors) {
        try {
            bool added = WorldUtils::add_sensor_to_world(p, world, SensorType::ON_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);
            if (!added) {
                std::cout << "On bridge sensor of size " << p.size() << " omitted\n";
            }
        } catch (const std::exception& e) {
            throw WorldCreationException(std::string("Error adding on bridge sensor: ") +
                            std::string(e.what()));
        }
    }

    auto under_bridge_sensors = WorldUtils::load_polygons_from_json("server/resources/world/under_bridge_sensors_liberty_city.json");
    for (const auto& p: under_bridge_sensors) {
        try {
            bool added = WorldUtils::add_sensor_to_world(p, world, SensorType::UNDER_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);
            if (!added) {
                std::cout << "Under bridge sensor of size " << p.size() << " omitted\n";
            }
        } catch (const std::exception& e) {
            throw WorldCreationException(std::string("Error adding under bridge sensor: ") +
                            std::string(e.what()));
        }
    }

    //cout info races
    std::cout << "Selected races: " << std::endl;
    for (const auto& race : selected_races.races) {
        std::cout << " - " << race.name << " in " << race.city << " with "
                  << static_cast<int>(race.size_checkpoints) << " checkpoints and "
                  << static_cast<int>(race.laps) << " laps." << std::endl;
    }
}


void GameLoop::remove_player(const uint32_t client_id) {

    const auto it = players.find(client_id);

    if (it != players.end())
        players.erase(it);

    game_monitor.unregister_client(client_id);

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << client_id
              << " has been removed from the Game World." << std::endl;
}

bool GameLoop::is_full() const { return players.size() >= max_players; }

bool GameLoop::is_empty() const { return players.empty(); }

uint8_t GameLoop::get_active_players() const {
    return static_cast<uint8_t>(players.size());
}


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

void GameLoop::handle_finish_race(const GameCommand& game_command) {

    if (auto stats = game_command.get_data<PlayerRaceStatsDTO>()) {
        uint32_t client_id = game_command.get_client_id();
        auto it = players.find(client_id);
        if (it != players.end()) {
            it->second.finish_race();
        }
        //update actual_race leaderboard
        
    }

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

void GameLoop::send_race(const RaceDTO& race){
    game_monitor.broadcast(race);
}


bool GameLoop::all_player_in_game() const {

    return std::all_of(players.begin(), players.end(),
                       [](const auto& p) { return p.second.is_in_game(); });
}

bool GameLoop::player_in_race() const {

    return std::any_of(players.begin(), players.end(),
                       [](const auto& p) { return p.second.is_in_race(); });
}

void GameLoop::send_finish_race_timeout() {
    game_monitor.broadcast(std::monostate{});
}

void GameLoop::start_race(const RaceDTO& race){    
    
    race_active = true;
    send_race(race);
    //client can upgrade their car, 10 seconds to do it
    //sleep 10 seconds?
    //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    
    for (auto& [client_id, player]: players) {
        //setting the positions of the players at the start line (first checkpoint)
        player.set_position_for_race(race.checkpoints);
        player.start_race();
    }
    
    //set timeout, if 10 min passes the race is finished, send message to clients
    //auto race_start_time = std::chrono::steady_clock::now();
    //auto race_timeout = std::chrono::minutes(10);

    //the race start, countdown 3,2,1, go!
    //server sends countdown to players
    //sleep 3 seconds?
    //std::this_thread::sleep_for(std::chrono::milliseconds(3000));

}


void GameLoop::run() {

    try {

        const int game_frame_ms = static_cast<int>(1000.0f / config.app.game.game_fps);
        const int waiting_frame_ms = static_cast<int>(1000.0f / config.app.server.waiting_for_players_fps);
        uint8_t current_race_index = 0;
        
        while (should_keep_running()) {


            process_commands();

            if (all_player_in_game()) {

                //start races
                if (!race_active && current_race_index < selected_races.size){
                    start_race(selected_races.races[current_race_index]);
                }
                
                if(player_in_race()){
                    /*auto current_time = std::chrono::steady_clock::now();
                    if (current_time - race_start_time >= race_timeout) {
                        send_finish_race_timeout();
                        break;
                    }*/
                    //race logic

                    update_positions();
                    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
                    std::this_thread::sleep_for(std::chrono::milliseconds(game_frame_ms));

                    //all players finish the race, calculate points in actual_race leaderboard
                    //calculate points and positions

                    //server send leaderboard to players, actual_race
                    //push_back actual race into leaderboards, update final_leaderboard and reset actual_race
                    //sleep to show leaderboard for some seconds, 5 seconds?
                }              
                
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
