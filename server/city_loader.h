#ifndef CITY_H
#define CITY_H


#include <string>
#include <vector>

#include "city_utils.h"


class CityLoader {

    b2World& box2d_world;
    const ServerConfig& server_config;
    CityUtils city_utils;

    bool city_loaded = false;
    std::string normalize_city_name(const std::string& city_name) const;
    const CityLoaderConfig& obtain_city_loader_config(
            const std::string& city_normalized_name) const;

    void load_collidables(const std::string& file, uint16_t collision_category, uint16_t collision_mask);
    void load_sensors(const std::string& file, SensorType type, uint16_t collision_category, uint16_t collision_mask);
    void load_checkpoints(const std::vector<CheckpointDTO>& checkpoints, uint16_t collision_category, uint16_t collision_mask);

public:
    CityLoader(const CityLoader&) = delete;
    CityLoader& operator=(const CityLoader&) = delete;

    explicit CityLoader(b2World& box2d_world, const ServerConfig& server_config);

    void load_city(const std::string& city_name, const std::vector<CheckpointDTO>& checkpoints);
};


#endif
