#ifndef NPC_H
#define NPC_H

#include <cmath>
#include <cstdint>
#include <vector>

#include "../common/npc_dto.h"

struct Point {
    float x;
    float y;

    Point(float x, float y): x(x), y(y) {}
};

class Npc {
private:
    uint32_t npc_id;
    std::vector<Point> route;  // Closed polygon route
    size_t current_segment;    // Current segment index (from route[current_segment] to
                               // route[current_segment+1])
    float progress;            // Progress along current segment [0.0, 1.0]
    float speed;               // Movement speed in pixels per update

    float current_x;
    float current_y;
    float current_angle;

    void update_angle();
    void advance_on_route();

public:
    Npc(uint32_t npc_id, const std::vector<std::vector<int>>& polygon, float speed);

    void update();
    NpcDTO get_position() const;
};

#endif  // NPC_H
