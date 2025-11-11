#include "config_loader.h"
#include <yaml-cpp/yaml.h>
#include <stdexcept>
#include <iostream>

ServerConfig ConfigLoader::load_yaml(const std::string& filepath) {
    try {
        YAML::Node config = YAML::LoadFile(filepath);
        
        if (!config["app"]) {
            throw std::runtime_error("Missing 'app' root node in configuration file");
        }

        ServerConfig serverConfig;
        YAML::Node app = config["app"];

        // Load game config
        if (app["game"]) {
            YAML::Node game = app["game"];
            serverConfig.app.game.game_fps = game["game_fps"].as<float>();
            serverConfig.app.game.max_concurrent_matches = game["max_concurrent_matches"].as<int>();
            serverConfig.app.game.min_races_per_match = game["min_races_per_match"].as<int>();
            serverConfig.app.game.max_races_per_match = game["max_races_per_match"].as<int>();
            serverConfig.app.game.min_cars_per_match = game["min_cars_per_match"].as<int>();
            serverConfig.app.game.max_cars_per_match = game["max_cars_per_match"].as<int>();
            serverConfig.app.game.wait_time_between_races_seconds = game["wait_time_between_races_seconds"].as<float>();
        }

        // Load server config
        if (app["server"]) {
            YAML::Node server = app["server"];
            serverConfig.app.server.command_queue_capacity = server["command_queue_capacity"].as<int>();
            serverConfig.app.server.waiting_for_players_fps = server["waiting_for_players_fps"].as<float>();

            // Load world config
            if (server["world"]) {
                YAML::Node world = server["world"];
                serverConfig.app.server.world.pixels_per_meter = world["pixels_per_meter"].as<float>();

                // Load collisions
                if (world["collisions"]) {
                    YAML::Node collisions = world["collisions"];
                    
                    if (collisions["liberty_city"]) {
                        YAML::Node lc = collisions["liberty_city"];
                        serverConfig.app.server.world.collisions.liberty_city.blocks_file = lc["blocks_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.liberty_city.bridges_file = lc["bridges_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.liberty_city.decorations_file = lc["decorations_file"].as<std::string>();
                    }

                    if (collisions["san_andreas"]) {
                        YAML::Node sa = collisions["san_andreas"];
                        serverConfig.app.server.world.collisions.san_andreas.blocks_file = sa["blocks_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.san_andreas.bridges_file = sa["bridges_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.san_andreas.decorations_file = sa["decorations_file"].as<std::string>();
                    }

                    if (collisions["vice_city"]) {
                        YAML::Node vc = collisions["vice_city"];
                        serverConfig.app.server.world.collisions.vice_city.blocks_file = vc["blocks_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.vice_city.bridges_file = vc["bridges_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.vice_city.decorations_file = vc["decorations_file"].as<std::string>();
                    }
                }

                // Load sprites
                if (world["sprites"] && world["sprites"]["car"]) {
                    YAML::Node car = world["sprites"]["car"];
                    serverConfig.app.server.world.sprites.car.width = car["width"].as<float>();
                    serverConfig.app.server.world.sprites.car.height = car["height"].as<float>();
                }

                // Load positions
                if (world["positions"] && world["positions"]["race_start"]) {
                    YAML::Node race_start = world["positions"]["race_start"];
                    serverConfig.app.server.world.positions.race_start.meters_x = race_start["meters_x"].as<float>();
                    serverConfig.app.server.world.positions.race_start.meters_y = race_start["meters_y"].as<float>();
                }

                // Load physics
                if (world["physics"]) {
                    YAML::Node physics = world["physics"];
                    
                    if (physics["block"]) {
                        YAML::Node block = physics["block"];
                        serverConfig.app.server.world.physics.block.density = block["density"].as<float>();
                        serverConfig.app.server.world.physics.block.friction = block["friction"].as<float>();
                        serverConfig.app.server.world.physics.block.restitution = block["restitution"].as<float>();
                    }

                    if (physics["car"]) {
                        YAML::Node car = physics["car"];
                        serverConfig.app.server.world.physics.car.density = car["density"].as<float>();
                        serverConfig.app.server.world.physics.car.friction = car["friction"].as<float>();
                        serverConfig.app.server.world.physics.car.restitution = car["restitution"].as<float>();
                        serverConfig.app.server.world.physics.car.linear_damping = car["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.car.angular_damping = car["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.car.min_turn_speed = car["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.car.acceleration = car["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.car.breaking_factor = car["breaking_factor"].as<float>();
                        serverConfig.app.server.world.physics.car.max_speed = car["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.car.drift_factor = car["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.car.turn_torque = car["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.car.mass = car["mass"].as<float>();
                        serverConfig.app.server.world.physics.car.health = car["health"].as<int>();
                    }
                }
            }
        }

        // Load client config
        if (app["client"] && app["client"]["cars"]) {
            YAML::Node cars = app["client"]["cars"];
            
            if (cars["compact"]) {
                YAML::Node compact = cars["compact"];
                serverConfig.app.client.cars.compact.health = compact["health"].as<int>();
                serverConfig.app.client.cars.compact.handling = compact["handling"].as<int>();
                serverConfig.app.client.cars.compact.acceleration = compact["acceleration"].as<int>();
                serverConfig.app.client.cars.compact.speed = compact["speed"].as<int>();
            }

            if (cars["sport"]) {
                YAML::Node sport = cars["sport"];
                serverConfig.app.client.cars.sport.health = sport["health"].as<int>();
                serverConfig.app.client.cars.sport.handling = sport["handling"].as<int>();
                serverConfig.app.client.cars.sport.acceleration = sport["acceleration"].as<int>();
                serverConfig.app.client.cars.sport.speed = sport["speed"].as<int>();
            }

            if (cars["convertible"]) {
                YAML::Node convertible = cars["convertible"];
                serverConfig.app.client.cars.convertible.health = convertible["health"].as<int>();
                serverConfig.app.client.cars.convertible.handling = convertible["handling"].as<int>();
                serverConfig.app.client.cars.convertible.acceleration = convertible["acceleration"].as<int>();
                serverConfig.app.client.cars.convertible.speed = convertible["speed"].as<int>();
            }

            if (cars["sedan"]) {
                YAML::Node sedan = cars["sedan"];
                serverConfig.app.client.cars.sedan.health = sedan["health"].as<int>();
                serverConfig.app.client.cars.sedan.handling = sedan["handling"].as<int>();
                serverConfig.app.client.cars.sedan.acceleration = sedan["acceleration"].as<int>();
                serverConfig.app.client.cars.sedan.speed = sedan["speed"].as<int>();
            }

            if (cars["jeep"]) {
                YAML::Node jeep = cars["jeep"];
                serverConfig.app.client.cars.jeep.health = jeep["health"].as<int>();
                serverConfig.app.client.cars.jeep.handling = jeep["handling"].as<int>();
                serverConfig.app.client.cars.jeep.acceleration = jeep["acceleration"].as<int>();
                serverConfig.app.client.cars.jeep.speed = jeep["speed"].as<int>();
            }

            if (cars["rally"]) {
                YAML::Node rally = cars["rally"];
                serverConfig.app.client.cars.rally.health = rally["health"].as<int>();
                serverConfig.app.client.cars.rally.handling = rally["handling"].as<int>();
                serverConfig.app.client.cars.rally.acceleration = rally["acceleration"].as<int>();
                serverConfig.app.client.cars.rally.speed = rally["speed"].as<int>();
            }

            if (cars["truck"]) {
                YAML::Node truck = cars["truck"];
                serverConfig.app.client.cars.truck.health = truck["health"].as<int>();
                serverConfig.app.client.cars.truck.handling = truck["handling"].as<int>();
                serverConfig.app.client.cars.truck.acceleration = truck["acceleration"].as<int>();
                serverConfig.app.client.cars.truck.speed = truck["speed"].as<int>();
            }
        }

        return serverConfig;

    } catch (const YAML::Exception& e) {
        throw std::runtime_error(std::string("YAML parsing error: ") + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Configuration loading error: ") + e.what());
    }
}
