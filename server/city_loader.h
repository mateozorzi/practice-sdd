#ifndef CITY_LOADER_H
#define CITY_LOADER_H


#include <string>
#include <vector>

#include "city_utils.h"
#include "npc.h"


class CityLoader {


    b2World& box2d_world;
    const ServerConfig& server_config;
    CityUtils city_utils;
    std::vector<Npc> npcs;

    bool city_loaded = false;

    void load_collidables(const std::string& file, uint16_t collision_category,
                          uint16_t collision_mask);
    void load_sensors(const std::string& file, SensorType type, uint16_t collision_category,
                      uint16_t collision_mask);
    void load_checkpoints(const std::vector<CheckpointDTO>& checkpoints,
                          uint16_t collision_category, uint16_t collision_mask);
    void load_npcs(const std::string& city_name);

public:
    CityLoader(const CityLoader&) = delete;
    CityLoader& operator=(const CityLoader&) = delete;

    explicit CityLoader(b2World& box2d_world, const ServerConfig& server_config);

    void load_city(const std::string& city_name, const std::vector<CheckpointDTO>& checkpoints);

    std::vector<Npc>& get_npcs();
    void update_npcs();
};


#endif
