#ifndef WORLD_UTILS_H
#define WORLD_UTILS_h


#include <string>
#include <vector>

#include <box2d/box2d.h>

#include "server_config.h"


enum class SensorType {

    ON_BRIDGE,
    UNDER_BRIDGE,
};


class CityUtils {

    b2World& box2d_world;
    const ServerConfig& server_config;

    std::array<b2Vec2, 4> obtain_b2_vertices(const std::vector<std::vector<int>>& polygon) const;

    float pixels_to_meters(float pixels) const;

public:
    CityUtils(const CityUtils&) = delete;
    CityUtils& operator=(const CityUtils&) = delete;

    explicit CityUtils(b2World& box2d_world, const ServerConfig& server_config);

    std::vector<std::vector<std::vector<int>>> load_polygons_from_file(
            const std::string& file_path);

    void add_collidable(const std::vector<std::vector<int>>& polygon, uint16_t collision_cateogry,
                        uint16_t collision_mask);
    void add_sensor(const std::vector<std::vector<int>>& polygon, SensorType sensor_type,
                    uint16_t collision_cateogry, uint16_t collision_mask);

    void cleanup();
};


#endif
