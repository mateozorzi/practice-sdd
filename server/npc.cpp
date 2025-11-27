#include "npc.h"

#include <cmath>

constexpr float PI = 3.14159265358979323846f;

Npc::Npc(uint32_t npc_id, const std::vector<std::vector<int>>& polygon, float speed):
        npc_id(npc_id),
        route(),
        current_segment(0),
        progress(0.0f),
        speed(speed),
        current_x(0.0f),
        current_y(0.0f),
        current_angle(0.0f) {

    // Convert polygon to route points (clockwise closed loop)
    for (const auto& point: polygon) {
        if (point.size() >= 2) {
            route.emplace_back(static_cast<float>(point[0]), static_cast<float>(point[1]));
        }
    }

    // Initialize position at first point
    if (!route.empty()) {
        current_x = route[0].x;
        current_y = route[0].y;
        update_angle();
    }
}

void Npc::update_angle() {
    if (route.size() < 2) {
        current_angle = 0.0f;
        return;
    }

    size_t next_idx = (current_segment + 1) % route.size();

    float dx = route[next_idx].x - route[current_segment].x;
    float dy = route[next_idx].y - route[current_segment].y;

    // Calculate angle in radians, then convert to degrees
    float angle_rad = std::atan2(dy, dx);
    float angle_deg = angle_rad * (180.0f / PI);

    // Normalize to [0, 360)
    angle_deg = std::fmod(angle_deg, 360.0f);
    if (angle_deg < 0.0f) {
        angle_deg += 360.0f;
    }

    current_angle = angle_deg;
}

void Npc::advance_on_route() {
    if (route.size() < 2) {
        return;
    }

    size_t next_idx = (current_segment + 1) % route.size();

    // Calculate segment length
    float dx = route[next_idx].x - route[current_segment].x;
    float dy = route[next_idx].y - route[current_segment].y;
    float segment_length = std::sqrt(dx * dx + dy * dy);

    if (segment_length < 0.01f) {
        // Segment too short, move to next
        current_segment = next_idx;
        progress = 0.0f;
        return;
    }

    // Increment progress based on speed
    float progress_increment = speed / segment_length;
    progress += progress_increment;

    // If we've completed this segment
    while (progress >= 1.0f) {
        progress -= 1.0f;
        current_segment = next_idx;
        next_idx = (current_segment + 1) % route.size();

        // Recalculate for new segment
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
    if (route.size() < 2) {
        return;
    }

    advance_on_route();

    // Linear interpolation between current and next point
    size_t next_idx = (current_segment + 1) % route.size();

    current_x =
            route[current_segment].x + (route[next_idx].x - route[current_segment].x) * progress;
    current_y =
            route[current_segment].y + (route[next_idx].y - route[current_segment].y) * progress;

    update_angle();
}

NpcDTO Npc::get_position() const {
    return NpcDTO(npc_id, static_cast<uint32_t>(current_x), static_cast<uint32_t>(current_y),
                  static_cast<uint32_t>(current_angle));
}
