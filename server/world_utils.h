#include <vector>

#include "box2d/b2_world.h"


enum class SensorType {

    ON_BRIDGE,
    UNDER_BRIDGE,
};


class WorldUtils {


public:
    WorldUtils(const WorldUtils&) = delete;
    WorldUtils& operator=(const WorldUtils&) = delete;

    static std::vector<std::vector<std::vector<int>>> load_polygons_from_json(const char* jsonPath);
    static bool add_polygon_to_world(const std::vector<std::vector<int>>& polygon, b2World& world);
    static bool add_sensor_to_world(const std::vector<std::vector<int>>& polygon, b2World& world, SensorType type);
    static float meters_to_pixels(const float meters);
    static float pixels_to_meters(const float pixels);
};
