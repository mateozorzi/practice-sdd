#ifndef CITY_H
#define CITY_H


#include <string>

#include "city_utils.h"


class CityLoader {

    b2World& box2d_world;
    const ServerConfig& server_config;
    CityUtils city_utils;

    bool city_loaded = false;
    std::string normalize_city_name(const std::string& city_name) const;

    void load_collidables(const std::string&, uint16_t category, uint16_t mask);
    void load_sensors(const std::string&, SensorType, uint16_t category, uint16_t mask);

    void cleanup_city();

public:
    CityLoader(const CityLoader&) = delete;
    CityLoader& operator=(const CityLoader&) = delete;

    explicit CityLoader(b2World& box2d_world, const ServerConfig& server_config);

    void load_city(const std::string& city_name);
};


#endif
