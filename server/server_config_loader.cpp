#include "server_config_loader.h"

#include <stdexcept>

ServerConfig ServerConfigLoader::load_yaml(const std::string& base_path) {
    try {

        ServerConfig server_config;

        std::string game_path = base_path + "/game_config.yaml";
        YAML::Node game_config = YAML::LoadFile(game_path);
        load_game_config(game_config, server_config);

        std::string physics_path = base_path + "/physics_config.yaml";
        YAML::Node physics_config = YAML::LoadFile(physics_path);
        load_physics_config(physics_config, server_config);

        std::string world_path = base_path + "/world_config.yaml";
        YAML::Node world_config = YAML::LoadFile(world_path);
        load_world_config(world_config, server_config);

        return server_config;

    } catch (const YAML::Exception& e) {

        throw std::runtime_error(std::string("YAML parsing error: ") + e.what());
    } catch (const std::exception& e) {

        throw std::runtime_error(std::string("Configuration loading error: ") + e.what());
    }
}


void ServerConfigLoader::load_game_config(const YAML::Node& config, ServerConfig& server_config) {


    if (!config["game"])
        throw std::runtime_error("Missing 'game' root node in game configuration file");

    YAML::Node game = config["game"];

    server_config.game.game_fps = game["game_fps"].as<unsigned int>();
    server_config.game.wait_seconds_upgrade_shop =
            game["wait_seconds_upgrade_shop"].as<unsigned int>();
    server_config.game.wait_seconds_countdown = game["wait_seconds_countdown"].as<unsigned int>();
    server_config.game.timeout_seconds_race = game["timeout_seconds_race"].as<unsigned int>();
    server_config.game.wait_seconds_leaderboard =
            game["wait_seconds_leaderboard"].as<unsigned int>();
    server_config.game.wait_seconds_game_finished =
            game["wait_seconds_game_finished"].as<unsigned int>();
    server_config.game.penalty_per_upgrade = game["penalty_per_upgrade"].as<unsigned int>();

    if (game["upgrades"]) {

        YAML::Node upgrades = game["upgrades"];
        server_config.game.upgrades.health_upgrade = upgrades["health_upgrade"].as<float>();
        server_config.game.upgrades.speed_upgrade = upgrades["speed_upgrade"].as<float>();
        server_config.game.upgrades.acceleration_upgrade =
                upgrades["acceleration_upgrade"].as<float>();
        server_config.game.upgrades.handling_upgrade = upgrades["handling_upgrade"].as<float>();
    }

    if (config["server"]) {

        YAML::Node server = config["server"];
        server_config.server.server_port = server["server_port"].as<unsigned short>();
        server_config.server.max_concurrent_games =
                server["max_concurrent_games"].as<unsigned int>();
        server_config.server.min_races_per_game = server["min_races_per_game"].as<unsigned int>();
        server_config.server.max_races_per_game = server["max_races_per_game"].as<unsigned int>();
        server_config.server.min_players_per_game =
                server["min_players_per_game"].as<unsigned int>();
        server_config.server.max_players_per_game =
                server["max_players_per_game"].as<unsigned int>();
        server_config.server.command_queue_capacity =
                server["command_queue_capacity"].as<unsigned int>();
        server_config.server.snapshot_queue_capacity =
                server["snapshot_queue_capacity"].as<unsigned int>();
    }
}


void ServerConfigLoader::load_physics_config(const YAML::Node& config,
                                             ServerConfig& server_config) {

    if (!config["physics"])
        throw std::runtime_error("Missing 'physics' node in physics configuration file");

    YAML::Node physics = config["physics"];

    if (physics["collidable"]) {

        YAML::Node collidable = physics["collidable"];
        server_config.physics.block.density = collidable["density"].as<float>();
        server_config.physics.block.friction = collidable["friction"].as<float>();
        server_config.physics.block.restitution = collidable["restitution"].as<float>();
    }

    if (physics["race_start"]) {

        YAML::Node race_start = physics["race_start"];
        server_config.physics.positions.race_start.meters_x = race_start["meters_x"].as<float>();
        server_config.physics.positions.race_start.meters_y = race_start["meters_y"].as<float>();
    }

    if (physics["cars"]) {

        YAML::Node cars = physics["cars"];

        if (cars["compact"])
            load_car_physics(cars["compact"], server_config.physics.compact);
        if (cars["sport"])
            load_car_physics(cars["sport"], server_config.physics.sport);
        if (cars["convertible"])
            load_car_physics(cars["convertible"], server_config.physics.convertible);
        if (cars["sedan"])
            load_car_physics(cars["sedan"], server_config.physics.sedan);
        if (cars["jeep"])
            load_car_physics(cars["jeep"], server_config.physics.jeep);
        if (cars["rally"])
            load_car_physics(cars["rally"], server_config.physics.rally);
        if (cars["truck"])
            load_car_physics(cars["truck"], server_config.physics.truck);
    }
}

void ServerConfigLoader::load_car_physics(const YAML::Node& car_node, CarConfig& car_config) {

    car_config.model = car_node["model"].as<std::string>();
    car_config.width = car_node["width"].as<float>();
    car_config.height = car_node["height"].as<float>();
    car_config.density = car_node["density"].as<float>();
    car_config.friction = car_node["friction"].as<float>();
    car_config.restitution = car_node["restitution"].as<float>();
    car_config.linear_damping = car_node["linear_damping"].as<float>();
    car_config.angular_damping = car_node["angular_damping"].as<float>();
    car_config.min_turn_speed = car_node["min_turn_speed"].as<float>();
    car_config.acceleration = car_node["acceleration"].as<float>();
    car_config.braking_factor = car_node["braking_factor"].as<float>();
    car_config.max_speed = car_node["max_speed"].as<float>();
    car_config.drift_factor = car_node["drift_factor"].as<float>();
    car_config.turn_torque = car_node["turn_torque"].as<float>();
    car_config.mass = car_node["mass"].as<float>();
    car_config.health = car_node["health"].as<unsigned int>();
}

void ServerConfigLoader::load_world_config(const YAML::Node& config, ServerConfig& server_config) {

    if (!config["world"])
        throw std::runtime_error("Missing 'world' node in world configuration file");

    YAML::Node world = config["world"];

    server_config.world.pixels_per_meter = world["pixels_per_meter"].as<unsigned int>();

    server_config.world.races_folder = world["races_folder"].as<std::string>();


    load_city(world["liberty_city"], server_config.world.liberty_city);
    load_city(world["san_andreas"], server_config.world.san_andreas);
    load_city(world["vice_city"], server_config.world.vice_city);
}


void ServerConfigLoader::load_city(const YAML::Node& node, CityLoaderConfig& city) {

    if (node["collidables"]) {

        auto c = node["collidables"];
        city.collidables.on_ground_collidable_file = c["on_ground"].as<std::string>();
        city.collidables.on_bridge_collidable_file = c["on_bridge"].as<std::string>();
    }

    if (node["sensors"]) {

        auto s = node["sensors"];
        city.sensors.on_bridge_sensor_file = s["on_bridge"].as<std::string>();
        city.sensors.under_bridge_sensor_file = s["under_bridge"].as<std::string>();
    }

    if (node["object_rendering"]) {

        auto obj = node["object_rendering"];
        city.objects.bridge_file = obj["bridge"].as<std::string>();
        city.objects.decoration_file = obj["decoration"].as<std::string>();
    }
}
