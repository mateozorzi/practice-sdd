#ifndef POWER_UP_H
#define POWER_UP_H


#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include <box2d/box2d.h>

#include "../common/checkpoint_dto.h"
#include "../common/power_up_type.h"

#include "server_config.h"


class Player;

// Draws the type of a power-up every time it appears. Injected so tests can use a fixed sequence.
using PowerUpTypeGenerator = std::function<PowerUpType()>;
// Creates the generator of each game
using PowerUpTypeGeneratorFactory = std::function<PowerUpTypeGenerator()>;

// Uniform over Nitro, Repair and Shield
PowerUpTypeGenerator make_random_power_up_type_generator();


class PowerUp {


    PowerUpType type;
    float x;  // pixels, center of the sensor
    float y;
    bool active = true;
    uint32_t collected_by = 0;       // id of the player that collected it, 0 since it appeared
    double respawn_remaining = 0.0;  // seconds
    b2Body* body;


public:
    PowerUp(PowerUpType type, float x, float y, b2Body* body);

    PowerUpType get_type() const;
    float get_x() const;
    float get_y() const;
    bool is_active() const;
    uint32_t get_collected_by() const;
    const b2Body* get_body() const;
    b2Body* get_body();

    void collect(double respawn_cooldown, uint32_t player_id);
    // Counts down the respawn cooldown. Returns true when the power-up has to reappear.
    bool advance(double elapsed_seconds);
    void respawn(PowerUpType new_type);
};


// Power-ups of the current race. Owns their Box2D bodies.
class PowerUpManager {


    b2World& box2d_world;
    const ServerConfig& server_config;
    PowerUpTypeGenerator type_generator;
    std::vector<PowerUp> power_ups;

    b2Body* create_sensor(float x, float y, size_t index);


public:
    PowerUpManager(const PowerUpManager&) = delete;
    PowerUpManager& operator=(const PowerUpManager&) = delete;

    PowerUpManager(b2World& box2d_world, const ServerConfig& server_config,
                   PowerUpTypeGenerator type_generator);
    ~PowerUpManager();

    // Replaces the current power-ups with one in the middle of each odd stretch (1->2, 3->4, ...)
    void load(const std::vector<CheckpointDTO>& checkpoints);
    // Applies the power-up to the player if it is active and the player is alive
    bool try_collect(size_t index, Player& player);
    // Explicit time step: counts down the respawn cooldowns (no wall clock)
    void advance(double elapsed_seconds);
    void clear();

    const std::vector<PowerUp>& get_power_ups() const;
};


#endif
