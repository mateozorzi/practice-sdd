#include "streets_test_utils.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace {

// Every test grid covers (0..260, 0..260)
constexpr int TEST_GRID_CELLS = 27;

// Horizontal or vertical segment of street cells, endpoints included (pixels, multiples of 10)
struct Segment {
    int from_x;
    int from_y;
    int to_x;
    int to_y;
};

StreetsGrid make_grid(const std::vector<Segment>& segments) {
    std::vector<std::string> rows(TEST_GRID_CELLS, std::string(TEST_GRID_CELLS, '0'));

    for (const Segment& segment: segments) {
        for (int y = std::min(segment.from_y, segment.to_y);
             y <= std::max(segment.from_y, segment.to_y); y += TEST_CELL_SIZE) {
            for (int x = std::min(segment.from_x, segment.to_x);
                 x <= std::max(segment.from_x, segment.to_x); x += TEST_CELL_SIZE)
                rows[y / TEST_CELL_SIZE][x / TEST_CELL_SIZE] = '1';
        }
    }

    return StreetsGrid(TEST_CELL_SIZE, std::move(rows));
}

}  // namespace


StreetMap make_straight_street_map() { return StreetMap{make_grid({{0, 0, 200, 0}}), {}}; }

StreetMap make_l_street_map() {
    return StreetMap{make_grid({{0, 0, 200, 0}, {200, 0, 200, 200}}), {}};
}

StreetMap make_two_streets_map() {
    return StreetMap{make_grid({{0, 0, 200, 0}, {0, 100, 200, 100}}), {}};
}

StreetMap make_test_crossing_map() {
    std::vector<Segment> segments;
    for (int y = 110; y <= 150; y += TEST_CELL_SIZE) segments.push_back({0, y, 260, y});
    for (int x = 110; x <= 150; x += TEST_CELL_SIZE) segments.push_back({x, 0, x, 260});
    segments.push_back({260, 110, 260, 260});
    segments.push_back({110, 260, 260, 260});

    BridgeLayout bridges;
    bridges.bridges = {{100, 160, 100, 160}};
    bridges.on_bridge_sensors = {{40, 220, 110, 150}};
    bridges.under_bridge_sensors = {{110, 150, 40, 220}};
    bridges.on_bridge_walls = {{100, 160, 100, 100}, {100, 160, 160, 160}};

    return StreetMap{make_grid(segments), std::move(bridges)};
}

StreetMap make_water_bridge_map() {
    std::vector<Segment> segments;
    for (int y = 110; y <= 150; y += TEST_CELL_SIZE) segments.push_back({0, y, 260, y});

    BridgeLayout bridges;
    bridges.bridges = {{100, 160, 100, 160}};
    bridges.on_bridge_sensors = {{40, 220, 110, 150}};
    bridges.on_bridge_walls = {{100, 160, 100, 100}, {100, 160, 160, 160}};

    return StreetMap{make_grid(segments), std::move(bridges)};
}
