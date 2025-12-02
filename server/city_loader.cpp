#include "city_loader.h"

#include <algorithm>
#include <iostream>

#include "../common/constants.h"

#include "world_creation_exception.h"


CityLoader::CityLoader(b2World& box2d_world, const ServerConfig& server_config): box2d_world(box2d_world), server_config(server_config), city_utils(box2d_world, server_config) {}


std::string CityLoader::normalize_city_name(const std::string& city_name) const {

    std::string modified_city_name = city_name;

    std::transform(modified_city_name.begin(), modified_city_name.end(), modified_city_name.begin(),
                   ::tolower);

    std::replace(modified_city_name.begin(), modified_city_name.end(), ' ', '_');

    return modified_city_name;
}


void CityLoader::load_collidables(const std::string& file, uint16_t category, uint16_t mask) {

    auto polygons = city_utils.load_polygons_from_file(file);

    for (const auto& polygon: polygons) {
        try {

            city_utils.add_collidable(polygon, category, mask);
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding collidable: " + std::string(e.what()));
        }
    }
}

void CityLoader::load_sensors(const std::string& file, SensorType type, uint16_t category,
                              uint16_t mask) {

    auto sensors = city_utils.load_polygons_from_file(file);

    for (const auto& poly: sensors) {
        try {

            city_utils.add_sensor(poly, type, category, mask);
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding sensor: " + std::string(e.what()));
        }
    }
}


void CityLoader::cleanup_city() {

    city_utils.cleanup();

    std::cout << "[CITY] Cleaning up previous city..." << std::endl;
}


void CityLoader::load_city(const std::string& city_name) {

    std::string city_normalized_name = normalize_city_name(city_name);

    if (city_loaded)
        cleanup_city();

    const CityLoaderConfig& city_loader_config =
        (city_normalized_name == "liberty_city") ? server_config.world.liberty_city :
        (city_normalized_name == "san_andreas") ? server_config.world.san_andreas :
        (city_normalized_name == "vice_city") ? server_config.world.vice_city :
        throw std::runtime_error("Unknown city: " + city_normalized_name);

    load_collidables(city_loader_config.collidables.on_ground_collidable_file, CATEGORY_GROUND,
                     MASK_GROUND);
    load_collidables(city_loader_config.collidables.on_ground_collidable_file, CATEGORY_BRIDGE,
                     MASK_BRIDGE);

    load_sensors(city_loader_config.sensors.on_bridge_sensor_file, SensorType::ON_BRIDGE,
                 CATEGORY_SENSOR, MASK_SENSOR);
    load_sensors(city_loader_config.sensors.under_bridge_sensor_file, SensorType::UNDER_BRIDGE,
                 CATEGORY_SENSOR, MASK_SENSOR);

    city_loaded = true;

    std::cout << "[CITY] Loaded world for city: " << city_name << std::endl << std::endl;
}
