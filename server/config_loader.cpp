#include "config_loader.h"

#include <iostream>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

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
            serverConfig.app.game.wait_time_between_races_seconds =
                    game["wait_time_between_races_seconds"].as<float>();
        }

        // Load server config
        if (app["server"]) {
            YAML::Node server = app["server"];
            serverConfig.app.server.command_queue_capacity =
                    server["command_queue_capacity"].as<int>();
            serverConfig.app.server.snapshot_queue_capacity =
                    server["snapshot_queue_capacity"].as<int>();
            serverConfig.app.server.waiting_for_players_fps =
                    server["waiting_for_players_fps"].as<float>();

            // Load world config
            if (server["world"]) {
                YAML::Node world = server["world"];
                serverConfig.app.server.world.pixels_per_meter =
                        world["pixels_per_meter"].as<float>();

                // Load collisions
                if (world["collisions"]) {
                    YAML::Node collisions = world["collisions"];

                    if (collisions["liberty_city"]) {
                        YAML::Node lc = collisions["liberty_city"];
                        serverConfig.app.server.world.collisions.liberty_city.blocks_file =
                                lc["blocks_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.liberty_city.bridges_file =
                                lc["bridges_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.liberty_city.decorations_file =
                                lc["decorations_file"].as<std::string>();
                    }

                    if (collisions["san_andreas"]) {
                        YAML::Node sa = collisions["san_andreas"];
                        serverConfig.app.server.world.collisions.san_andreas.blocks_file =
                                sa["blocks_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.san_andreas.bridges_file =
                                sa["bridges_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.san_andreas.decorations_file =
                                sa["decorations_file"].as<std::string>();
                    }

                    if (collisions["vice_city"]) {
                        YAML::Node vc = collisions["vice_city"];
                        serverConfig.app.server.world.collisions.vice_city.blocks_file =
                                vc["blocks_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.vice_city.bridges_file =
                                vc["bridges_file"].as<std::string>();
                        serverConfig.app.server.world.collisions.vice_city.decorations_file =
                                vc["decorations_file"].as<std::string>();
                    }
                }

                // Load positions
                if (world["positions"] && world["positions"]["race_start"]) {
                    YAML::Node race_start = world["positions"]["race_start"];
                    serverConfig.app.server.world.positions.race_start.meters_x =
                            race_start["meters_x"].as<float>();
                    serverConfig.app.server.world.positions.race_start.meters_y =
                            race_start["meters_y"].as<float>();
                }

                // Load physics
                if (world["physics"]) {
                    YAML::Node physics = world["physics"];

                    if (physics["block"]) {
                        YAML::Node block = physics["block"];
                        serverConfig.app.server.world.physics.block.density =
                                block["density"].as<float>();
                        serverConfig.app.server.world.physics.block.friction =
                                block["friction"].as<float>();
                        serverConfig.app.server.world.physics.block.restitution =
                                block["restitution"].as<float>();
                    }

                    // Load individual car type physics (compact, sport, etc.)
                    if (physics["compact"]) {
                        YAML::Node compact = physics["compact"];
                        serverConfig.app.server.world.physics.compact.width = compact["width"].as<float>();
                        serverConfig.app.server.world.physics.compact.height = compact["height"].as<float>();
                        serverConfig.app.server.world.physics.compact.density = compact["density"].as<float>();
                        serverConfig.app.server.world.physics.compact.friction = compact["friction"].as<float>();
                        serverConfig.app.server.world.physics.compact.restitution = compact["restitution"].as<float>();
                        serverConfig.app.server.world.physics.compact.linear_damping = compact["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.compact.angular_damping = compact["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.compact.min_turn_speed = compact["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.compact.acceleration = compact["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.compact.braking_factor = compact["braking_factor"].as<float>();
                        serverConfig.app.server.world.physics.compact.max_speed = compact["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.compact.drift_factor = compact["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.compact.turn_torque = compact["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.compact.mass = compact["mass"].as<float>();
                        serverConfig.app.server.world.physics.compact.health = compact["health"].as<int>();
                    }

                    if (physics["sport"]) {
                        YAML::Node sport = physics["sport"];
                        serverConfig.app.server.world.physics.sport.width = sport["width"].as<float>();
                        serverConfig.app.server.world.physics.sport.height = sport["height"].as<float>();
                        serverConfig.app.server.world.physics.sport.density = sport["density"].as<float>();
                        serverConfig.app.server.world.physics.sport.friction = sport["friction"].as<float>();
                        serverConfig.app.server.world.physics.sport.restitution = sport["restitution"].as<float>();
                        serverConfig.app.server.world.physics.sport.linear_damping = sport["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.sport.angular_damping = sport["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.sport.min_turn_speed = sport["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.sport.acceleration = sport["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.sport.braking_factor = sport["braking_factor"].as<float>();
                        serverConfig.app.server.world.physics.sport.max_speed = sport["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.sport.drift_factor = sport["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.sport.turn_torque = sport["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.sport.mass = sport["mass"].as<float>();
                        serverConfig.app.server.world.physics.sport.health = sport["health"].as<int>();
                    }

                    if (physics["convertible"]) {
                        YAML::Node convertible = physics["convertible"];
                        serverConfig.app.server.world.physics.convertible.width = convertible["width"].as<float>();
                        serverConfig.app.server.world.physics.convertible.height = convertible["height"].as<float>();
                        serverConfig.app.server.world.physics.convertible.density = convertible["density"].as<float>();
                        serverConfig.app.server.world.physics.convertible.friction = convertible["friction"].as<float>();
                        serverConfig.app.server.world.physics.convertible.restitution = convertible["restitution"].as<float>();
                        serverConfig.app.server.world.physics.convertible.linear_damping = convertible["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.convertible.angular_damping = convertible["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.convertible.min_turn_speed = convertible["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.convertible.acceleration = convertible["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.convertible.braking_factor = convertible["braking_factor"].as<float>();
                        serverConfig.app.server.world.physics.convertible.max_speed = convertible["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.convertible.drift_factor = convertible["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.convertible.turn_torque = convertible["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.convertible.mass = convertible["mass"].as<float>();
                        serverConfig.app.server.world.physics.convertible.health = convertible["health"].as<int>();
                    }

                    if (physics["sedan"]) {
                        YAML::Node sedan = physics["sedan"];
                        serverConfig.app.server.world.physics.sedan.width = sedan["width"].as<float>();
                        serverConfig.app.server.world.physics.sedan.height = sedan["height"].as<float>();
                        serverConfig.app.server.world.physics.sedan.density = sedan["density"].as<float>();
                        serverConfig.app.server.world.physics.sedan.friction = sedan["friction"].as<float>();
                        serverConfig.app.server.world.physics.sedan.restitution = sedan["restitution"].as<float>();
                        serverConfig.app.server.world.physics.sedan.linear_damping = sedan["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.sedan.angular_damping = sedan["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.sedan.min_turn_speed = sedan["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.sedan.acceleration = sedan["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.sedan.braking_factor = sedan["braking_factor"].as<float>();
                        serverConfig.app.server.world.physics.sedan.max_speed = sedan["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.sedan.drift_factor = sedan["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.sedan.turn_torque = sedan["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.sedan.mass = sedan["mass"].as<float>();
                        serverConfig.app.server.world.physics.sedan.health = sedan["health"].as<int>();
                    }

                    if (physics["jeep"]) {
                        YAML::Node jeep = physics["jeep"];
                        serverConfig.app.server.world.physics.jeep.width = jeep["width"].as<float>();
                        serverConfig.app.server.world.physics.jeep.height = jeep["height"].as<float>();
                        serverConfig.app.server.world.physics.jeep.density = jeep["density"].as<float>();
                        serverConfig.app.server.world.physics.jeep.friction = jeep["friction"].as<float>();
                        serverConfig.app.server.world.physics.jeep.restitution = jeep["restitution"].as<float>();
                        serverConfig.app.server.world.physics.jeep.linear_damping = jeep["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.jeep.angular_damping = jeep["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.jeep.min_turn_speed = jeep["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.jeep.acceleration = jeep["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.jeep.braking_factor = jeep["braking_factor"].as<float>();
                        serverConfig.app.server.world.physics.jeep.max_speed = jeep["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.jeep.drift_factor = jeep["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.jeep.turn_torque = jeep["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.jeep.mass = jeep["mass"].as<float>();
                        serverConfig.app.server.world.physics.jeep.health = jeep["health"].as<int>();
                    }

                    if (physics["rally"]) {
                        YAML::Node rally = physics["rally"];
                        serverConfig.app.server.world.physics.rally.width = rally["width"].as<float>();
                        serverConfig.app.server.world.physics.rally.height = rally["height"].as<float>();
                        serverConfig.app.server.world.physics.rally.density = rally["density"].as<float>();
                        serverConfig.app.server.world.physics.rally.friction = rally["friction"].as<float>();
                        serverConfig.app.server.world.physics.rally.restitution = rally["restitution"].as<float>();
                        serverConfig.app.server.world.physics.rally.linear_damping = rally["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.rally.angular_damping = rally["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.rally.min_turn_speed = rally["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.rally.acceleration = rally["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.rally.braking_factor = rally["braking_factor"].as<float>();
                        serverConfig.app.server.world.physics.rally.max_speed = rally["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.rally.drift_factor = rally["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.rally.turn_torque = rally["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.rally.mass = rally["mass"].as<float>();
                        serverConfig.app.server.world.physics.rally.health = rally["health"].as<int>();
                    }

                    if (physics["truck"]) {
                        YAML::Node truck = physics["truck"];
                        serverConfig.app.server.world.physics.truck.width = truck["width"].as<float>();
                        serverConfig.app.server.world.physics.truck.height = truck["height"].as<float>();
                        serverConfig.app.server.world.physics.truck.density = truck["density"].as<float>();
                        serverConfig.app.server.world.physics.truck.friction = truck["friction"].as<float>();
                        serverConfig.app.server.world.physics.truck.restitution = truck["restitution"].as<float>();
                        serverConfig.app.server.world.physics.truck.linear_damping = truck["linear_damping"].as<float>();
                        serverConfig.app.server.world.physics.truck.angular_damping = truck["angular_damping"].as<float>();
                        serverConfig.app.server.world.physics.truck.min_turn_speed = truck["min_turn_speed"].as<float>();
                        serverConfig.app.server.world.physics.truck.acceleration = truck["acceleration"].as<float>();
                        serverConfig.app.server.world.physics.truck.braking_factor = truck["braking_factor"].as<float>();
                        serverConfig.app.server.world.physics.truck.max_speed = truck["max_speed"].as<float>();
                        serverConfig.app.server.world.physics.truck.drift_factor = truck["drift_factor"].as<float>();
                        serverConfig.app.server.world.physics.truck.turn_torque = truck["turn_torque"].as<float>();
                        serverConfig.app.server.world.physics.truck.mass = truck["mass"].as<float>();
                        serverConfig.app.server.world.physics.truck.health = truck["health"].as<int>();
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
                serverConfig.app.client.cars.compact.acceleration =
                        compact["acceleration"].as<int>();
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
                serverConfig.app.client.cars.convertible.handling =
                        convertible["handling"].as<int>();
                serverConfig.app.client.cars.convertible.acceleration =
                        convertible["acceleration"].as<int>();
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
