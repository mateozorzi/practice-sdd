#ifndef CITY_LOADER_H
#define CITY_LOADER_H


#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "city_utils.h"
#include "npc.h"
#include "streets_grid.h"


class CityLoader {


    b2World& box2d_world;
    const ServerConfig& server_config;
    CityUtils city_utils;
    std::vector<Npc> npcs;
    // Loaded the first time a race runs in each city; nullopt if the city has no grid
    std::unordered_map<std::string, std::optional<StreetMap>> street_maps;
    const StreetMap* street_map = nullptr;

    bool city_loaded = false;

    void load_collidables(const std::string& file, uint16_t collision_category,
                          uint16_t collision_mask);
    void load_sensors(const std::string& file, SensorType type, uint16_t collision_category,
                      uint16_t collision_mask);
    void load_checkpoints(const std::vector<CheckpointDTO>& checkpoints,
                          uint16_t collision_category, uint16_t collision_mask);
    void load_npcs(const std::string& city_name);
    const StreetMap* load_street_map_once(const std::string& normalized_city_name,
                                          const CityLoaderConfig& city_loader_config);


public:
    CityLoader(const CityLoader&) = delete;
    CityLoader& operator=(const CityLoader&) = delete;

    explicit CityLoader(b2World& box2d_world, const ServerConfig& server_config);

    void load_city(const std::string& city_name, const std::vector<CheckpointDTO>& checkpoints);

    // Street grid and bridges of the current city, nullptr if it has none (see design D7)
    const StreetMap* get_street_map() const;

    std::vector<Npc>& get_npcs();
    void update_npcs();
};


#endif
