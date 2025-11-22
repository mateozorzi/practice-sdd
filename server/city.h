#ifndef CITY_H
#define CITY_H


#include "world_utils.h"


class City {

public:
    City(const City&) = delete;
    City& operator=(const City&) = delete;

    City() = default;

    void load_city(b2World& world, const std::string& city_name);

private:
    std::string normalize_city_name(const std::string& city_name) const;

    std::string obtain_ground_collidables_path(const std::string& city_name) const;
    std::string obtain_bridge_collidables_path(const std::string& city_name) const;
    std::string obtain_on_bridge_sensors_path(const std::string& city_name) const;
    std::string obtain_under_bridge_sensors_path(const std::string& city_name) const;

    void load_collidables(b2World&, const std::string&, uint16_t category, uint16_t mask);
    void load_sensors(b2World&, const std::string&, SensorType, uint16_t category, uint16_t mask);

    void cleanup_city(b2World& world);
};


#endif