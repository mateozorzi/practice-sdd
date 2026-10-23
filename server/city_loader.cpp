#include "city_loader.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "../common/constants.h"

#include "world_creation_exception.h"


CityLoader::CityLoader(b2World& box2d_world, const ServerConfig& server_config):
        box2d_world(box2d_world), server_config(server_config), city_utils(server_config) {}


void CityLoader::load_collidables(const std::string& file, const uint16_t collision_category,
                                  const uint16_t collision_mask) {

    auto polygons = city_utils.load_polygons_from_file(file);

    for (const auto& polygon: polygons) {

        try {

            city_utils.add_collidable(box2d_world, polygon, collision_category, collision_mask);
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding collidable: " + std::string(e.what()));
        }
    }
}

void CityLoader::load_sensors(const std::string& file, SensorType type,
                              const uint16_t collision_category, const uint16_t collision_mask) {

    auto sensors = city_utils.load_polygons_from_file(file);

    for (const auto& sensor: sensors) {

        try {

            city_utils.add_sensor(box2d_world, sensor, type, collision_category, collision_mask);
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding sensor: " + std::string(e.what()));
        }
    }
}

void CityLoader::load_npcs(const std::string& city_name) {

    npcs.clear();

    std::string normalized_city_name = city_utils.normalize_city_name(city_name);

    const CityLoaderConfig& city_loader_config =
            city_utils.obtain_city_loader_config(normalized_city_name);

    std::string npc_file = city_loader_config.npc_file;

    try {
        std::ifstream file(npc_file);
        if (!file)
            return;

        nlohmann::json json_data;
        file >> json_data;

        if (!json_data.is_array())
            return;

        uint32_t npc_id = 0;
        constexpr float NPC_SPEED = 1.5f;

        for (const auto& polygon: json_data) {
            if (polygon.is_array() && polygon.size() >= 3) {
                std::vector<std::vector<int>> route;
                for (const auto& point: polygon) {
                    if (point.is_array() && point.size() >= 2) {
                        route.push_back({point[0].get<int>(), point[1].get<int>()});
                    }
                }
                npcs.emplace_back(npc_id++, route, NPC_SPEED, box2d_world, server_config);
            }
        }

    } catch (const std::exception& e) {

        throw WorldCreationException("Error adding npcs: " + std::string(e.what()));
    }
}

void CityLoader::load_checkpoints(const std::vector<CheckpointDTO>& checkpoints,
                                  const uint16_t collision_category,
                                  const uint16_t collision_mask) {

    for (const auto& checkpoint: checkpoints) {

        try {

            city_utils.add_checkpoint(box2d_world, checkpoint, collision_category, collision_mask);
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding checkpoint: " + std::string(e.what()));
        }
    }
}


void CityLoader::load_city(const std::string& city_name,
                           const std::vector<CheckpointDTO>& checkpoints) {

    std::string normalized_city_name = city_utils.normalize_city_name(city_name);

    if (city_loaded)
        city_utils.cleanup(box2d_world);

    const CityLoaderConfig& city_loader_config =
            city_utils.obtain_city_loader_config(normalized_city_name);

    load_collidables(city_loader_config.collidables.on_ground_collidable_file, CATEGORY_GROUND,
                     MASK_GROUND);
    load_collidables(city_loader_config.collidables.on_bridge_collidable_file, CATEGORY_BRIDGE,
                     MASK_BRIDGE);

    load_sensors(city_loader_config.sensors.on_bridge_sensor_file, SensorType::ON_BRIDGE,
                 CATEGORY_SENSOR, MASK_SENSOR);
    load_sensors(city_loader_config.sensors.under_bridge_sensor_file, SensorType::UNDER_BRIDGE,
                 CATEGORY_SENSOR, MASK_SENSOR);

    load_npcs(city_name);

    street_map = load_street_map_once(normalized_city_name, city_loader_config);

    load_checkpoints(checkpoints, CATEGORY_SENSOR, MASK_SENSOR);

    city_loaded = true;
}

const StreetMap* CityLoader::load_street_map_once(const std::string& normalized_city_name,
                                                  const CityLoaderConfig& city_loader_config) {

    auto [entry, inserted] = street_maps.try_emplace(normalized_city_name);

    // A missing or broken grid is logged once and the race goes on: the power-ups fall back to
    // the straight midpoint
    if (inserted && !city_loader_config.streets_file.empty()) {

        try {

            entry->second.emplace(load_street_map(city_loader_config, city_utils));
        } catch (const std::exception& e) {

            std::cerr << "[" << POWER_UPS << "] No street grid for " << normalized_city_name << ": "
                      << e.what() << std::endl;
        }
    }

    return entry->second ? &*entry->second : nullptr;
}

const StreetMap* CityLoader::get_street_map() const { return street_map; }

std::vector<Npc>& CityLoader::get_npcs() { return npcs; }

void CityLoader::update_npcs() {

    for (auto& npc: npcs) npc.update();
}
