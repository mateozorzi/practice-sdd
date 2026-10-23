#ifndef STREETS_GRID_H
#define STREETS_GRID_H


#include <optional>
#include <string>
#include <vector>

#include "../common/car_position_dto.h"

#include "server_config.h"


class CityUtils;


// Pixels of the race
struct StreetPoint {
    float x;
    float y;
};

struct StreetCell {
    int col;
    int row;

    bool operator==(const StreetCell& other) const = default;
};

// Axis-aligned box in pixels, edges included
struct StreetRect {
    float min_x;
    float max_x;
    float min_y;
    float max_y;
};


// Street cells of a city, generated once from its B_STREETS mask. The cell (col, row) has its
// center at (col * cell_size, row * cell_size) and covers half a cell to each side.
class StreetsGrid {


    int cell_size;
    int width;
    int height;
    std::vector<std::string> rows;  // '1' street, '0' not street


public:
    // Throws WorldCreationException if the rows do not match a positive cell size and the
    // dimensions
    StreetsGrid(int cell_size, std::vector<std::string> rows);

    int get_cell_size() const;
    int get_width() const;
    int get_height() const;

    // False outside the grid
    bool is_street(int col, int row) const;
    StreetPoint cell_center(int col, int row) const;
    // Cell that contains the point, nullopt outside the grid
    std::optional<StreetCell> cell_at(StreetPoint point) const;
    // Street cell whose center is closest to the point (also for points outside the grid)
    std::optional<StreetCell> nearest_street_cell(StreetPoint point) const;
};


// Bridges of a city, as boxes of the polygons of its world files
struct BridgeLayout {
    std::vector<StreetRect> bridges;
    std::vector<StreetRect> on_bridge_sensors;
    std::vector<StreetRect> under_bridge_sensors;
    std::vector<StreetRect> on_bridge_walls;  // rails: they only stop the cars on the bridge
};

struct StreetMap {
    StreetsGrid grid;
    BridgeLayout bridges;
};

struct PathStep {
    int col;
    int row;
    CarLayer layer;
};


// Reads streets_<city>.json: {"cell_size", "width", "height", "rows"}. Throws
// WorldCreationException with the path and the problem.
StreetsGrid load_streets_grid(const std::string& path);

// Grid of the city plus the bridges, sensors and rails of its world files
StreetMap load_street_map(const CityLoaderConfig& city_config, CityUtils& city_utils);

// Shortest path along the streets between the street cells closest to `from` and `to`. It carries
// the layer of a car (same rules as CollisionListener) and never turns inside a bridge.
std::optional<std::vector<PathStep>> find_street_path(const StreetMap& street_map, StreetPoint from,
                                                      StreetPoint to);

// Point of the path (through the cell centers) that leaves half of its length on each side
StreetPoint path_midpoint(const StreetsGrid& grid, const std::vector<PathStep>& path);


#endif
