#ifndef WORLD_UTILS_H
#define WORLD_UTILS_h


#include <string>
#include <vector>
#include <box2d/box2d.h>


enum class SensorType {

    ON_BRIDGE,
    UNDER_BRIDGE,
};


class WorldUtils {

    b2World& world;

    std::array<b2Vec2, 4> obtain_b2_vertices(const std::vector<std::vector<int>>& polygon) const;

    float meters_to_pixels(float meters) const;
    float pixels_to_meters(float pixels) const;

public:
    WorldUtils(const WorldUtils&) = delete;
    WorldUtils& operator=(const WorldUtils&) = delete;

    explicit WorldUtils(b2World& world);

    std::vector<std::vector<std::vector<int>>> load_polygons_from_file(const std::string& file_path);

    void add_collidable(const std::vector<std::vector<int>>& polygon, uint16_t category, uint16_t mask);
    void add_sensor (const std::vector<std::vector<int>>& polygon, SensorType type, uint16_t category, uint16_t mask);

    void cleanup();
};


#endif
