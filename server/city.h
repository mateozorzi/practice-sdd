#ifndef CITY_H
#define CITY_H


#include "world_utils.h"


class City {

    b2World& world;
    WorldUtils utils;
    std::string normalize_city_name(const std::string& city_name) const;

    std::string obtain_ground_collidables_path(const std::string& city_name) const;
    std::string obtain_bridge_collidables_path(const std::string& city_name) const;
    std::string obtain_on_bridge_sensors_path(const std::string& city_name) const;
    std::string obtain_under_bridge_sensors_path(const std::string& city_name) const;

    void load_collidables(const std::string&, uint16_t category, uint16_t mask);
    void load_sensors(const std::string&, SensorType, uint16_t category, uint16_t mask);

    void cleanup_city();

public:
    City(const City&) = delete;
    City& operator=(const City&) = delete;

    explicit City(b2World& world);

    void load_city(const std::string& city_name);
};


#endif