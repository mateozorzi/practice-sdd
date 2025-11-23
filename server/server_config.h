#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <string>

// Innermost structs first (no dependencies)
struct CarClientConfig {
    float health;
    float handling;
    float acceleration;
    float speed;
};

struct CarDefaults {
    struct CarClientConfig compact;
    struct CarClientConfig sport;
    struct CarClientConfig convertible;
    struct CarClientConfig sedan;
    struct CarClientConfig jeep;
    struct CarClientConfig rally;
    struct CarClientConfig truck;
};

struct BlockPhysicsConfig {
    float density;
    float friction;
    float restitution;
};

struct CarPhysicsConfig {

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
    int health;
};

struct PhysicsConfig {

    struct BlockPhysicsConfig block;
    struct CarPhysicsConfig compact;
    struct CarPhysicsConfig sport;
    struct CarPhysicsConfig convertible;
    struct CarPhysicsConfig sedan;
    struct CarPhysicsConfig jeep;
    struct CarPhysicsConfig rally;
    struct CarPhysicsConfig truck;
};

struct CoordinatesConfig {
    float meters_x;
    float meters_y;
};

struct PositionsConfig {
    struct CoordinatesConfig race_start;
};

struct WorldCollisionsConfig {
    std::string blocks_file;
    std::string bridges_file;
    std::string decorations_file;
};

struct CollisionsConfig {
    struct WorldCollisionsConfig liberty_city;
    struct WorldCollisionsConfig san_andreas;
    struct WorldCollisionsConfig vice_city;
};

struct WorldConfig {
    float pixels_per_meter;
    struct CollisionsConfig collisions;
    struct PositionsConfig positions;
    struct PhysicsConfig physics;
};

struct UpgradeConfig {
    float health_upgrade;
    float speed_upgrade;
    float acceleration_upgrade;
    float handling_upgrade;
};

// game = both server and client
struct GameConfig {
    float game_fps;
    unsigned int max_concurrent_matches;
    unsigned int min_races_per_match;
    unsigned int max_races_per_match;
    unsigned int min_cars_per_match;
    unsigned int max_cars_per_match;
    float wait_time_between_races_seconds;
    struct UpgradeConfig upgrades;
};

struct ClientSpecificConfig {
    struct CarDefaults cars;
};

struct ServerSpecificConfig {
    // unsigned short      default_server_port;
    float command_queue_capacity;
    float snapshot_queue_capacity;
    float waiting_for_players_fps;
    struct WorldConfig world;
};

struct ServerConfigDef {
    struct GameConfig game;
    struct ServerSpecificConfig server;
    struct ClientSpecificConfig client;
};

struct ServerConfig {
    struct ServerConfigDef app;
};

#endif  // SERVER_CONFIG_H
