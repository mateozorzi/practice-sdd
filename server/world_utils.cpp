#include "car.h"
#include "world_utils.h"

#include <fstream>
#include <iostream>
#include <string>

#include <box2d/box2d.h>
#include <nlohmann/json.hpp>

#include "../common/constants.h"

#include "world_creation_exception.h"

bool WorldUtils::add_polygon_to_world(const std::vector<std::vector<int>>& polygon,
                                      b2World& world, const uint16_t category,
                                      const uint16_t mask) {

    if (polygon.size() < 3) {
        throw WorldCreationException("Invalid polygon vertex count: " + polygon.size());
    }

    // TODO: de momento los vamos a ignorar
    if (polygon.size() > b2_maxPolygonVertices) {
        return false;
    }

    // Convert the polygon coordinates to b2Vec2 array
    b2Vec2 vertices[4];
    for (size_t i = 0; i < polygon.size(); i++) {
        if (polygon[i].size() < 2) {
            throw WorldCreationException("Invalid vertex format");
        }
        float x = pixels_to_meters(polygon[i][0]);
        float y = pixels_to_meters(polygon[i][1]);
        vertices[i].Set(x, y);
    }

    // Create the polygon shape
    b2PolygonShape polygonShape;
    polygonShape.Set(vertices, static_cast<int32>(polygon.size()));

    // Create a static body definition
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;

    // Create the body in the world
    b2Body* body = world.CreateBody(&bodyDef);

    // Create a fixture definition
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &polygonShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.0f;
    fixtureDef.restitution = 0.0f;

    fixtureDef.filter.categoryBits = category;
    fixtureDef.filter.maskBits = mask;

    // Attach the fixture to the body
    body->CreateFixture(&fixtureDef);

    return true;
}


bool WorldUtils::add_sensor_to_world(const std::vector<std::vector<int>>& polygon, 
                                       b2World& world, 
                                       SensorType type, const uint16_t category, const uint16_t mask) {

    b2Vec2 vertices[4];
    for (size_t i = 0; i < polygon.size(); i++) {
        if (polygon[i].size() < 2) {
            throw WorldCreationException("Invalid vertex format");
        }
        float x = pixels_to_meters(polygon[i][0]);
        float y = pixels_to_meters(polygon[i][1]);
        vertices[i].Set(x, y);
    }

    b2PolygonShape sensorShape;
    sensorShape.Set(vertices, static_cast<int32>(polygon.size()));

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;

    b2Body* body = world.CreateBody(&bodyDef);

    //body->GetUserData().pointer = (uintptr_t)type;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &sensorShape;
    fixtureDef.isSensor = true;

    fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(new SensorType(type));

    fixtureDef.filter.categoryBits = category;
    fixtureDef.filter.maskBits = mask;

    body->CreateFixture(&fixtureDef);

    return true;
}


std::vector<std::vector<std::vector<int>>> WorldUtils::load_polygons_from_json(
        const char* jsonPath) {
    std::vector<std::vector<std::vector<int>>> polygons;

    // Open the JSON file
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        throw WorldCreationException("Could not open JSON file");
    }

    // Parse the JSON
    nlohmann::json jsonData;
    try {
        file >> jsonData;
    } catch (const nlohmann::json::exception& e) {
        throw WorldCreationException(std::string("JSON parse error: ") + std::string(e.what()));
    }

    // Extract polygons from JSON
    // Assuming the JSON structure is an array of polygons, where each polygon is an array of
    // vertices Example: [[[x1, y1], [x2, y2], [x3, y3]], [[x4, y4], [x5, y5], [x6, y6]]]
    if (!jsonData.is_array()) {
        throw WorldCreationException("JSON root must be an array of polygons");
    }

    for (const auto& polygon: jsonData) {
        if (!polygon.is_array()) {
            throw WorldCreationException("Each polygon must be an array of vertices");
        }

        std::vector<std::vector<int>> polyVertices;
        for (const auto& vertex: polygon) {
            if (!vertex.is_array() || vertex.size() < 2) {
                throw WorldCreationException(
                        "Each vertex must be an array with at least 2 coordinates");
            }

            std::vector<int> coords;
            coords.push_back(vertex[0].get<int>());
            coords.push_back(vertex[1].get<int>());
            polyVertices.push_back(coords);
        }
        polygons.push_back(polyVertices);
    }

    return polygons;
}

float WorldUtils::meters_to_pixels(const float meters) { return meters * PIXELS_PER_METER; }

float WorldUtils::pixels_to_meters(const float pixels) { return pixels / PIXELS_PER_METER; }
