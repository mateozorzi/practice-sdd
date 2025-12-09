#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H


#include <string>


// ============================================================================
// PHYSICS CONFIGURATION STRUCTS
// ============================================================================


struct CollidableConfig {


    float density;
    float friction;
    float restitution;
};


struct CarConfig {


    std::string model;
    float width;
    float height;
    float density;
    float friction;
    float restitution;
    float linear_damping;   // Sliding effect after stopping moving (lower = more sliding)
    float angular_damping;  // The same that above, but when turning (lower = more turning)
    float min_turn_speed;
    float acceleration;
    float braking_factor;
    float max_speed;
    float drift_factor;     // The lower = more drifting
    float turn_torque;
    float mass;
    unsigned int health;
};


struct RaceStartPosition {


    float meters_x;
    float meters_y;
};


struct PositionsConfig {


    struct RaceStartPosition race_start;
};


struct PhysicsConfig {


    struct CollidableConfig block;
    struct PositionsConfig positions;
    struct CarConfig compact;
    struct CarConfig sport;
    struct CarConfig convertible;
    struct CarConfig sedan;
    struct CarConfig jeep;
    struct CarConfig rally;
    struct CarConfig truck;
};


// ============================================================================
// WORLD CONFIGURATION STRUCTS
// ============================================================================


struct CollidableFiles {


    std::string on_ground_collidable_file;
    std::string on_bridge_collidable_file;
};


struct SensorFiles {


    std::string on_bridge_sensor_file;
    std::string under_bridge_sensor_file;
};

struct ObjectRenderingFiles {

    std::string bridge_file;
    std::string decoration_file;
};


struct CityLoaderConfig {


    struct CollidableFiles collidables;
    struct SensorFiles sensors;
    struct ObjectRenderingFiles objects;
    std::string npc_file;
};


struct WorldConfig {


    unsigned int pixels_per_meter;
    std::string races_folder;
    struct CityLoaderConfig liberty_city;
    struct CityLoaderConfig san_andreas;
    struct CityLoaderConfig vice_city;
};


// ============================================================================
// GAME CONFIGURATION STRUCTS
// ============================================================================


struct UpgradeConfig {


    float health_upgrade;
    float speed_upgrade;
    float acceleration_upgrade;
    float handling_upgrade;
};


struct GameConfig {


    unsigned int game_fps;
    unsigned int wait_seconds_upgrade_shop;
    unsigned int wait_seconds_countdown;
    unsigned int timeout_seconds_race;
    unsigned int wait_seconds_leaderboard;
    unsigned int wait_seconds_game_finished;
    unsigned int penalty_per_upgrade;

    struct UpgradeConfig upgrades;
};


struct ServerSpecificConfig {


    unsigned short server_port;
    unsigned int max_concurrent_games;
    unsigned int min_races_per_game;
    unsigned int max_races_per_game;
    unsigned int min_players_per_game;
    unsigned int max_players_per_game;
    unsigned int command_queue_capacity;
    unsigned int snapshot_queue_capacity;
};


// ============================================================================
// ROOT CONFIGURATION STRUCT
// ============================================================================


struct ServerConfig {


    struct PhysicsConfig physics;

    struct WorldConfig world;

    struct GameConfig game;
    struct ServerSpecificConfig server;
};


#endif
