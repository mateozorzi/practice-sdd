#include "npc.h"

#include <cmath>

constexpr float PI = 3.14159265358979323846f;

Npc::Npc(uint32_t npc_id, const std::vector<std::vector<int>>& polygon, float speed,
         b2World& box2d_world, const ServerConfig& server_config):
        npc_id(npc_id),
        speed(speed),
        server_config(server_config) {

    for (const auto& point: polygon) {
        if (point.size() >= 2) {
            route.emplace_back(static_cast<float>(point[0]), static_cast<float>(point[1]));
        }
    }

    if (route.empty())
        return;

    b2BodyDef box2d_body_definition;
    box2d_body_definition.type = b2_kinematicBody;
    box2d_body_definition.position.Set(pixels_to_meters(route[0].x), pixels_to_meters(route[0].y));
    box2d_body_definition.angle = 0.0f;

    box2d_body = box2d_world.CreateBody(&box2d_body_definition);

    b2PolygonShape box2d_polygon_shape;
    box2d_polygon_shape.SetAsBox(server_config.physics.compact.width, server_config.physics.compact.height);

    b2FixtureDef box2d_fixture_definition;
    box2d_fixture_definition.shape = &box_shape;
    box2d_fixture_definition.density = 1.0f;

    box2d_body->CreateFixture(&fixture_def);

    update_angle();
}

float Npc::pixels_to_meters(float pixels) const {
    return pixels / server_config.world.pixels_per_meter;
}

void Npc::update_angle() {
    if (route.size() < 2) {
        current_angle = 0.0f;
        return;
    }

    size_t next_idx = (current_segment + 1) % route.size();

    float dx = route[next_idx].x - route[current_segment].x;
    float dy = route[next_idx].y - route[current_segment].y;

    float angle_rad = std::atan2(dy, dx);
    float angle_deg = angle_rad * (180.0f / PI);

    angle_deg = std::fmod(angle_deg, 360.0f);
    if (angle_deg < 0.0f) {
        angle_deg += 360.0f;
    }

    current_angle = angle_deg;
}

void Npc::advance_on_route() {

    if (route.size() < 2)
        return;

    size_t next_idx = (current_segment + 1) % route.size();

    float dx = route[next_idx].x - route[current_segment].x;
    float dy = route[next_idx].y - route[current_segment].y;
    float segment_length = std::sqrt(dx * dx + dy * dy);

    if (segment_length < 0.01f) {
        current_segment = next_idx;
        progress = 0.0f;
        return;
    }

    float progress_increment = speed / segment_length;
    progress += progress_increment;

    while (progress >= 1.0f) {
        progress -= 1.0f;
        current_segment = next_idx;
        next_idx = (current_segment + 1) % route.size();

        dx = route[next_idx].x - route[current_segment].x;
        dy = route[next_idx].y - route[current_segment].y;
        segment_length = std::sqrt(dx * dx + dy * dy);

        if (segment_length < 0.01f) {
            progress = 0.0f;
            break;
        }
    }
}

void Npc::update() {

    if (route.size() < 2 || !box2d_body)
        return;

    advance_on_route();

    size_t next_idx = (current_segment + 1) % route.size();
    float current_x = route[current_segment].x + 
                      (route[next_idx].x - route[current_segment].x) * progress;
    float current_y = route[current_segment].y + 
                      (route[next_idx].y - route[current_segment].y) * progress;

    update_angle();

    box2d_body->SetTransform(b2Vec2(pixels_to_meters(current_x), pixels_to_meters(current_y)),
                       current_angle * (PI / 180.0f));
}

NpcDTO Npc::get_position() const {

    if (!box2d_body)
        return NpcDTO(npc_id, 0, 0, 0);

    b2Vec2 pos = box2d_body->GetPosition();
    float angle = box2d_body->GetAngle() * (180.0f / PI);

    uint32_t x = static_cast<uint32_t>(pos.x * server_config.world.pixels_per_meter);
    uint32_t y = static_cast<uint32_t>(pos.y * server_config.world.pixels_per_meter);
    uint32_t angle_deg = static_cast<uint32_t>(angle);

    return NpcDTO(npc_id, x, y, angle_deg);
}

b2Body* Npc::get_body() const {
    return box2d_body;
}

