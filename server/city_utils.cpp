#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


#include "../common/constants.h"


#include "city_utils.h"
#include "world_creation_exception.h"


CityUtils::CityUtils(b2World& box2d_world) : box2d_world(box2d_world) {}


std::array<b2Vec2, 4> CityUtils::obtain_b2_vertices(const std::vector<std::vector<int>>& polygon) const {

    if (polygon.size() != 4)
        throw WorldCreationException("Polygons must have exactly 4 vertices");

    std::array<b2Vec2, 4> vertices{};

    for (size_t i = 0; i < 4; i++) {
        if (polygon[i].size() < 2)
            throw WorldCreationException("Invalid vertex format");

        vertices[i].Set(
            pixels_to_meters(polygon[i][0]),
            pixels_to_meters(polygon[i][1])
        );
    }

    return vertices;
}


float CityUtils::meters_to_pixels(const float meters) const { return meters * PIXELS_PER_METER; }

float CityUtils::pixels_to_meters(const float pixels) const { return pixels / PIXELS_PER_METER; }


std::vector<std::vector<std::vector<int>>> CityUtils::load_polygons_from_file(const std::string& file_path) {

    std::ifstream file(file_path);
    if (!file)
        throw WorldCreationException("Could not open file");

    nlohmann::json json_data;
    file >> json_data;

    if (!json_data.is_array())
        throw WorldCreationException("JSON root must be an array");

    std::vector<std::vector<std::vector<int>>> polygons;

    for (const auto& polygon: json_data) {

        if (!polygon.is_array() || polygon.size() != 4)
            throw WorldCreationException("Each polygon must have exactly 4 vertices");

        std::vector<std::vector<int>> polygon_vertices;
        for (const auto& vertex: polygon) {
            polygon_vertices.push_back({ vertex[0].get<int>(), vertex[1].get<int>() });
        }
        polygons.push_back(polygon_vertices);
    }

    return polygons;
}


void CityUtils::add_collidable(const std::vector<std::vector<int>>& polygon, const uint16_t collision_category, const uint16_t collision_mask) {

    auto vertices = obtain_b2_vertices(polygon);

    b2PolygonShape box2d_polygon_shape;
    box2d_polygon_shape.Set(vertices.data(), 4);

    b2BodyDef box2d_body_definition;
    box2d_body_definition.type = b2_staticBody;
    b2Body* box2d_body = box2d_world.CreateBody(&box2d_body_definition);

    b2FixtureDef box2d_fixture_definition;
    box2d_fixture_definition.shape = &box2d_polygon_shape;
    box2d_fixture_definition.filter.categoryBits = collision_category;
    box2d_fixture_definition.filter.maskBits = collision_mask;

    box2d_body->CreateFixture(&box2d_fixture_definition);
}

void CityUtils::add_sensor(const std::vector<std::vector<int>>& polygon, const SensorType type, const uint16_t collision_category, const uint16_t collision_mask) {

    auto vertices = obtain_b2_vertices(polygon);

    b2PolygonShape box2d_polygon_shape;
    box2d_polygon_shape.Set(vertices.data(), 4);

    b2BodyDef box2d_body_definition;
    box2d_body_definition.type = b2_staticBody;
    b2Body* box2d_body = box2d_world.CreateBody(&box2d_body_definition);

    b2FixtureDef box2d_fixture_definition;
    box2d_fixture_definition.shape = &box2d_polygon_shape;
    box2d_fixture_definition.isSensor = true;
    box2d_fixture_definition.userData.pointer = static_cast<uintptr_t>(type);
    box2d_fixture_definition.filter.categoryBits = collision_category;
    box2d_fixture_definition.filter.maskBits = collision_mask;

    box2d_body->CreateFixture(&box2d_fixture_definition);
}


void CityUtils::cleanup() {

    std::vector<b2Body*> bodies_to_destroy;

    for (b2Body* box2d_body = box2d_world.GetBodyList(); box2d_body; box2d_body = box2d_body->GetNext()) {

        if (box2d_body->GetType() == b2_staticBody)
            bodies_to_destroy.push_back(box2d_body);
    }

    for (auto* box2d_body: bodies_to_destroy)
        box2d_world.DestroyBody(box2d_body);
}
