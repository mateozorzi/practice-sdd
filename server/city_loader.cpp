#include "car.h" // Should remove later
#include "city_loader.h"
#include "world_creation_exception.h"


#include <algorithm>
#include <iostream>


CityLoader::CityLoader(b2World& box2d_world) : box2d_world(box2d_world), city_utils(box2d_world) {}


std::string CityLoader::normalize_city_name(const std::string& city_name) const {

    std::string modified_city_name = city_name;
    std::transform(modified_city_name.begin(), modified_city_name.end(), modified_city_name.begin(), ::tolower);
    std::replace(modified_city_name.begin(), modified_city_name.end(), ' ', '_');

    return modified_city_name;
}


std::string CityLoader::obtain_ground_collidables_path(const std::string& city_name) const {

    return "server/resources/world/collidables/on_ground_collidables_" + city_name + ".json";
}

std::string CityLoader::obtain_bridge_collidables_path(const std::string& city_name) const {

    return "server/resources/world/collidables/on_bridge_collidables_" + city_name + ".json";
}

std::string CityLoader::obtain_on_bridge_sensors_path(const std::string& city_name) const {

    return "server/resources/world/sensors/on_bridge_sensors_" + city_name + ".json";
}

std::string CityLoader::obtain_under_bridge_sensors_path(const std::string& city_name) const {

    return "server/resources/world/sensors/under_bridge_sensors_" + city_name + ".json";
}

void CityLoader::load_collidables(const std::string& file, uint16_t category, uint16_t mask) {

    auto polygons = city_utils.load_polygons_from_file(file.c_str());

    for (const auto& polygon: polygons) {
        try {

            city_utils.add_collidable(polygon, category, mask);
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding collidable: " + std::string(e.what()));
        }
    }
}

void CityLoader::load_sensors(const std::string& file, SensorType type, uint16_t category, uint16_t mask) {

    auto sensors = city_utils.load_polygons_from_file(file.c_str());

    for (const auto& poly : sensors) {
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

    cleanup_city();

    load_collidables(obtain_ground_collidables_path(city_normalized_name), CATEGORY_GROUND, MASK_GROUND);
    load_collidables(obtain_bridge_collidables_path(city_normalized_name), CATEGORY_BRIDGE, MASK_BRIDGE);

    load_sensors(obtain_on_bridge_sensors_path(city_normalized_name), SensorType::ON_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);
    load_sensors(obtain_under_bridge_sensors_path(city_normalized_name), SensorType::UNDER_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);

    std::cout << "[CITY] Loaded world for city: " << city_name << std::endl << std::endl;
}
