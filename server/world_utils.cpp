#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


#include "../common/constants.h"


#include "world_utils.h"
#include "world_creation_exception.h"


WorldUtils::WorldUtils(b2World& world) : world(world) {}


std::array<b2Vec2, 4> WorldUtils::obtain_b2_vertices(const std::vector<std::vector<int>>& polygon) const {

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


float WorldUtils::meters_to_pixels(const float meters) const { return meters * PIXELS_PER_METER; }

float WorldUtils::pixels_to_meters(const float pixels) const { return pixels / PIXELS_PER_METER; }


std::vector<std::vector<std::vector<int>>> WorldUtils::load_polygons_from_file(const std::string& file_path) {

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


void WorldUtils::add_collidable(const std::vector<std::vector<int>>& polygon, const uint16_t category, const uint16_t mask) {

    auto vertices = obtain_b2_vertices(polygon);

    b2PolygonShape shape;
    shape.Set(vertices.data(), 4);

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    b2Body* body = world.CreateBody(&bodyDef);

    b2FixtureDef fdef;
    fdef.shape = &shape;
    fdef.filter.categoryBits = category;
    fdef.filter.maskBits = mask;

    body->CreateFixture(&fdef);
}

void WorldUtils::add_sensor(const std::vector<std::vector<int>>& polygon, const SensorType type, const uint16_t category, const uint16_t mask) {

    auto vertices = obtain_b2_vertices(polygon);

    b2PolygonShape shape;
    shape.Set(vertices.data(), 4);

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    b2Body* body = world.CreateBody(&bodyDef);

    b2FixtureDef fdef;
    fdef.shape = &shape;
    fdef.isSensor = true;
    fdef.userData.pointer = static_cast<uintptr_t>(type);
    fdef.filter.categoryBits = category;
    fdef.filter.maskBits = mask;

    body->CreateFixture(&fdef);
}


void WorldUtils::cleanup() {

    std::vector<b2Body*> bodies_to_destroy;

    for (b2Body* body = world.GetBodyList(); body; body = body->GetNext()) {

        if (body->GetType() == b2_staticBody)
            bodies_to_destroy.push_back(body);
    }

    for (auto* body : bodies_to_destroy)
        world.DestroyBody(body);
}
