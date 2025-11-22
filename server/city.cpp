#include "car.h" // Should remove later
#include "city.h"
#include "world_creation_exception.h"


#include <algorithm>
#include <iostream>


std::string City::normalize_city_name(const std::string& city_name) const {

    std::string modified_city_name = city_name;
    std::transform(modified_city_name.begin(), modified_city_name.end(), modified_city_name.begin(), ::tolower);
    std::replace(modified_city_name.begin(), modified_city_name.end(), ' ', '_');

    return modified_city_name;
}


std::string City::obtain_ground_collidables_path(const std::string& city_name) const {

    return "server/resources/world/collidables/on_ground_collidables_" + city_name + ".json";
}

std::string City::obtain_bridge_collidables_path(const std::string& city_name) const {

    return "server/resources/world/collidables/on_bridge_collidables_" + city_name + ".json";
}

std::string City::obtain_on_bridge_sensors_path(const std::string& city_name) const {

    return "server/resources/world/sensors/on_bridge_sensors_" + city_name + ".json";
}

std::string City::obtain_under_bridge_sensors_path(const std::string& city_name) const {

    return "server/resources/world/sensors/under_bridge_sensors_" + city_name + ".json";
}

void City::load_collidables(
        b2World& world,
        const std::string& file,
        uint16_t category,
        uint16_t mask
) {

    WorldUtils utils(world);

    auto polygons = utils.load_polygons_from_file(file.c_str());

    for (const auto& poly : polygons) {
        try {

            bool added = utils.add_collidable(poly, category, mask);
            (void)added; // Should remove later!
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding collidable: " + std::string(e.what()));
        }
    }
}

void City::load_sensors(
        b2World& world,
        const std::string& file,
        SensorType type,
        uint16_t category,
        uint16_t mask
) {

    WorldUtils utils(world);

    auto sensors = utils.load_polygons_from_file(file.c_str());

    for (const auto& poly : sensors) {
        try {

            bool added = utils.add_sensor(poly, type, category, mask);
            (void)added; // Should remove later!
        } catch (const std::exception& e) {

            throw WorldCreationException("Error adding sensor: " + std::string(e.what()));
        }
    }
}


void City::cleanup_city(b2World& world) {

    WorldUtils utils(world);

    utils.cleanup();

    std::cout << "[CITY] Cleaning up previous city..." << std::endl;
}


void City::load_city(b2World& world, const std::string& city_name) {

    std::string city_normalized_name = normalize_city_name(city_name);

    cleanup_city(world);

    load_collidables(world, obtain_ground_collidables_path(city_normalized_name), CATEGORY_GROUND, MASK_GROUND);
    load_collidables(world, obtain_bridge_collidables_path(city_normalized_name), CATEGORY_BRIDGE, MASK_BRIDGE);

    load_sensors(world, obtain_on_bridge_sensors_path(city_normalized_name), SensorType::ON_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);
    load_sensors(world, obtain_under_bridge_sensors_path(city_normalized_name), SensorType::UNDER_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);

    std::cout << "[CITY] Loaded world for city: " << city_name << std::endl << std::endl;
}
