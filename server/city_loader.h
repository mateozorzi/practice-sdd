#ifndef CITY_H
#define CITY_H


#include <string>

#include "city_utils.h"


class CityLoader {

    b2World& box2d_world;
    CityUtils city_utils;
    std::string normalize_city_name(const std::string& city_name) const;

    std::string obtain_ground_collidables_path(const std::string& city_name) const;
    std::string obtain_bridge_collidables_path(const std::string& city_name) const;
    std::string obtain_on_bridge_sensors_path(const std::string& city_name) const;
    std::string obtain_under_bridge_sensors_path(const std::string& city_name) const;

    void load_collidables(const std::string&, uint16_t category, uint16_t mask);
    void load_sensors(const std::string&, SensorType, uint16_t category, uint16_t mask);

    void cleanup_city();

public:
    CityLoader(const CityLoader&) = delete;
    CityLoader& operator=(const CityLoader&) = delete;

    explicit CityLoader(b2World& box2d_world);

    void load_city(const std::string& city_name);
};


#endif
