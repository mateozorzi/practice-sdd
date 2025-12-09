#ifndef NPC_H
#define NPC_H


#include <cmath>
#include <cstdint>
#include <vector>

#include <box2d/box2d.h>

#include "../common/npc_dto.h"

#include "server_config.h"


struct Point {


    float x;
    float y;

    Point(float x, float y): x(x), y(y) {}
};


class Npc {


    uint32_t npc_id;
    std::vector<Point> route;    // Closed polygon route
    size_t current_segment = 0;  // Current segment index (from route[current_segment] to
                                 // route[current_segment+1])
    float progress = 0.0f;       // Progress along current segment [0.0, 1.0]
    float speed;                 // Movement speed in pixels per update

    b2Body* box2d_body = nullptr;
    const ServerConfig& server_config;

    float current_x = 0.0f;
    float current_y = 0.0f;
    float current_angle = 0.0f;

    float pixels_to_meters(const float pixels) const;
    void update_angle();
    void advance_on_route();


public:
    Npc(uint32_t npc_id, const std::vector<std::vector<int>>& polygon, float speed,
        b2World& box2d_world, const ServerConfig& server_config);

    void update();
    NpcDTO get_position() const;

    b2Body* get_body() const;
};


#endif
