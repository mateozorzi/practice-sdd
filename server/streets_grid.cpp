#include "streets_grid.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <utility>

#include <nlohmann/json.hpp>

#include "city_utils.h"
#include "world_creation_exception.h"


namespace {

constexpr char STREET_CELL = '1';
constexpr char NO_STREET_CELL = '0';

void validate_rows(const int cell_size, const std::vector<std::string>& rows) {

    if (cell_size <= 0)
        throw WorldCreationException("cell_size must be positive, got " +
                                     std::to_string(cell_size));

    const size_t width = rows.empty() ? 0 : rows.front().size();

    for (size_t row = 0; row < rows.size(); row++) {

        if (rows[row].size() != width)
            throw WorldCreationException("row " + std::to_string(row) + " has " +
                                         std::to_string(rows[row].size()) +
                                         " cells but the width is " + std::to_string(width));

        if (rows[row].find_first_not_of({STREET_CELL, NO_STREET_CELL}) != std::string::npos)
            throw WorldCreationException("row " + std::to_string(row) +
                                         " has a cell that is not '0' or '1'");
    }
}

// Moves to the 4 neighbors. The heading of a move is its index + 1; 0 means no heading.
constexpr std::array<std::array<int, 2>, 4> MOVES = {{{0, -1}, {0, 1}, {1, 0}, {-1, 0}}};
constexpr int NO_HEADING = 0;
constexpr int HEADINGS = 5;
constexpr int LAYERS = 3;

bool contains(const StreetRect& rect, const StreetPoint point) {
    return point.x >= rect.min_x && point.x <= rect.max_x && point.y >= rect.min_y &&
           point.y <= rect.max_y;
}

bool any_contains(const std::vector<StreetRect>& rects, const StreetPoint point) {
    return std::any_of(rects.begin(), rects.end(),
                       [point](const StreetRect& rect) { return contains(rect, point); });
}

// Open intervals, so a rail of height 0 at y = 100 only touches the cells centered on y = 100
bool overlaps_cell(const StreetRect& rect, const StreetPoint center, const float half_cell) {
    return rect.min_x < center.x + half_cell && rect.max_x > center.x - half_cell &&
           rect.min_y < center.y + half_cell && rect.max_y > center.y - half_cell;
}

// What the path needs to know about a cell
struct CellInfo {
    bool on_bridge_sensor = false;
    bool under_bridge_sensor = false;
    bool rail = false;
    int bridge = -1;  // index of the bridge rectangle that contains the center, -1 if none
};

// Describes each cell the first time the search reaches it: describing the whole city up front
// costs more than most searches
class CellDescriptions {

    const StreetsGrid& grid;
    const BridgeLayout& bridges;
    std::vector<CellInfo> cells;
    std::vector<bool> described;

    CellInfo describe(const int col, const int row) const {

        const StreetPoint center = grid.cell_center(col, row);
        const float half_cell = static_cast<float>(grid.get_cell_size()) / 2.0f;

        CellInfo info;
        info.on_bridge_sensor = any_contains(bridges.on_bridge_sensors, center);
        info.under_bridge_sensor = any_contains(bridges.under_bridge_sensors, center);
        info.rail = std::any_of(bridges.on_bridge_walls.begin(), bridges.on_bridge_walls.end(),
                                [center, half_cell](const StreetRect& wall) {
                                    return overlaps_cell(wall, center, half_cell);
                                });
        const auto bridge =
                std::find_if(bridges.bridges.begin(), bridges.bridges.end(),
                             [center](const StreetRect& rect) { return contains(rect, center); });
        if (bridge != bridges.bridges.end())
            info.bridge = static_cast<int>(bridge - bridges.bridges.begin());

        return info;
    }

public:
    explicit CellDescriptions(const StreetMap& street_map):
            grid(street_map.grid),
            bridges(street_map.bridges),
            cells(static_cast<size_t>(grid.get_width()) * grid.get_height()),
            described(cells.size(), false) {}

    const CellInfo& at(const int col, const int row) {

        const size_t index = static_cast<size_t>(row) * grid.get_width() + col;
        if (!described[index]) {
            cells[index] = describe(col, row);
            described[index] = true;
        }
        return cells[index];
    }

    size_t size() const { return cells.size(); }
};

// Same rules as CollisionListener: first the sensors it leaves, then the ones it enters, and only a
// car on the ground changes its layer when it enters a sensor
CarLayer next_layer(CarLayer layer, const CellInfo& from, const CellInfo& to) {

    if ((layer == CarLayer::ON_BRIDGE && from.on_bridge_sensor && !to.on_bridge_sensor) ||
        (layer == CarLayer::UNDER_BRIDGE && from.under_bridge_sensor && !to.under_bridge_sensor))
        layer = CarLayer::ON_GROUND;

    if (layer != CarLayer::ON_GROUND)
        return layer;

    if (!from.on_bridge_sensor && to.on_bridge_sensor)
        return CarLayer::ON_BRIDGE;

    if (!from.under_bridge_sensor && to.under_bridge_sensor)
        return CarLayer::UNDER_BRIDGE;

    return layer;
}

std::vector<CarLayer> start_layers(const CellInfo& start) {

    if (start.on_bridge_sensor && start.under_bridge_sensor)
        return {CarLayer::ON_BRIDGE, CarLayer::UNDER_BRIDGE};
    if (start.on_bridge_sensor)
        return {CarLayer::ON_BRIDGE};
    if (start.under_bridge_sensor)
        return {CarLayer::UNDER_BRIDGE};
    return {CarLayer::ON_GROUND};
}

// Bounding box of a polygon of the world files
StreetRect box_of(const std::vector<std::vector<int>>& polygon) {

    StreetRect box{0.0f, 0.0f, 0.0f, 0.0f};

    for (size_t vertex = 0; vertex < polygon.size(); vertex++) {

        const auto x = static_cast<float>(polygon[vertex].at(0));
        const auto y = static_cast<float>(polygon[vertex].at(1));

        if (vertex == 0) {
            box = {x, x, y, y};
            continue;
        }

        box.min_x = std::min(box.min_x, x);
        box.max_x = std::max(box.max_x, x);
        box.min_y = std::min(box.min_y, y);
        box.max_y = std::max(box.max_y, y);
    }

    return box;
}

int read_int(const nlohmann::json& json, const std::string& key) {

    if (!json.contains(key) || !json[key].is_number_integer())
        throw WorldCreationException("missing integer field " + key);

    return json[key].get<int>();
}

// True if the square of half side `half_size` centered in `point` is entirely inside a bridge
// (edges included), false if it is entirely outside all of them (touching an edge counts as
// outside), nullopt if it crosses the edge of one
std::optional<bool> inside_a_bridge(const std::vector<StreetRect>& bridges, const StreetPoint point,
                                    const float half_size) {

    bool inside = false;
    for (const StreetRect& bridge: bridges) {

        const bool contained =
                point.x - half_size >= bridge.min_x && point.x + half_size <= bridge.max_x &&
                point.y - half_size >= bridge.min_y && point.y + half_size <= bridge.max_y;
        const bool apart =
                point.x + half_size <= bridge.min_x || point.x - half_size >= bridge.max_x ||
                point.y + half_size <= bridge.min_y || point.y - half_size >= bridge.max_y;
        if (!contained && !apart)
            return std::nullopt;

        inside = inside || contained;
    }

    return inside;
}

// Layer of the step of the path whose cell contains the point at `distance` along it
CarLayer path_layer_at(const StreetsGrid& grid, const std::vector<PathStep>& path,
                       const float distance, const StreetPoint point) {

    const auto segment =
            std::min(static_cast<size_t>(distance / static_cast<float>(grid.get_cell_size())),
                     path.size() - 1);
    const std::optional<StreetCell> cell = grid.cell_at(point);

    if (cell && segment + 1 < path.size() && path[segment + 1].col == cell->col &&
        path[segment + 1].row == cell->row)
        return path[segment + 1].layer;

    return path[segment].layer;
}

}  // namespace


StreetsGrid::StreetsGrid(const int cell_size, std::vector<std::string> rows):
        cell_size(cell_size),
        width(rows.empty() ? 0 : static_cast<int>(rows.front().size())),
        height(static_cast<int>(rows.size())),
        rows(std::move(rows)) {

    validate_rows(this->cell_size, this->rows);
}

int StreetsGrid::get_cell_size() const { return cell_size; }

int StreetsGrid::get_width() const { return width; }

int StreetsGrid::get_height() const { return height; }

bool StreetsGrid::is_street(const int col, const int row) const {

    if (col < 0 || row < 0 || col >= width || row >= height)
        return false;

    return rows[row][col] == STREET_CELL;
}

StreetPoint StreetsGrid::cell_center(const int col, const int row) const {
    return {static_cast<float>(col * cell_size), static_cast<float>(row * cell_size)};
}

std::optional<StreetCell> StreetsGrid::cell_at(const StreetPoint point) const {

    const auto size = static_cast<float>(cell_size);
    const StreetCell cell{static_cast<int>(std::lround(point.x / size)),
                          static_cast<int>(std::lround(point.y / size))};

    if (cell.col < 0 || cell.row < 0 || cell.col >= width || cell.row >= height)
        return std::nullopt;

    return cell;
}

std::optional<StreetCell> StreetsGrid::nearest_street_cell(const StreetPoint point) const {

    std::optional<StreetCell> nearest;
    double nearest_distance = 0.0;

    // Row by row, so a tie keeps the lowest row and then the lowest column
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {

            if (!is_street(col, row))
                continue;

            const StreetPoint center = cell_center(col, row);
            const double dx = static_cast<double>(center.x) - point.x;
            const double dy = static_cast<double>(center.y) - point.y;
            const double distance = dx * dx + dy * dy;

            if (!nearest || distance < nearest_distance) {
                nearest = StreetCell{col, row};
                nearest_distance = distance;
            }
        }
    }

    return nearest;
}


StreetsGrid load_streets_grid(const std::string& path) {

    try {

        std::ifstream file(path);
        if (!file)
            throw WorldCreationException("could not open the file");

        const nlohmann::json json = nlohmann::json::parse(file);

        const int cell_size = read_int(json, "cell_size");
        const int width = read_int(json, "width");
        const int height = read_int(json, "height");

        if (!json.contains("rows") || !json["rows"].is_array())
            throw WorldCreationException("missing array field rows");

        std::vector<std::string> rows;
        for (const auto& row: json["rows"]) {

            if (!row.is_string())
                throw WorldCreationException("every row must be a string");

            rows.push_back(row.get<std::string>());
        }

        if (static_cast<int>(rows.size()) != height)
            throw WorldCreationException("height is " + std::to_string(height) + " but there are " +
                                         std::to_string(rows.size()) + " rows");

        const auto wrong_row = std::find_if(rows.begin(), rows.end(), [width](const auto& row) {
            return static_cast<int>(row.size()) != width;
        });
        if (wrong_row != rows.end())
            throw WorldCreationException("width is " + std::to_string(width) + " but row " +
                                         std::to_string(wrong_row - rows.begin()) + " has " +
                                         std::to_string(wrong_row->size()) + " cells");

        return StreetsGrid(cell_size, std::move(rows));

    } catch (const std::exception& e) {

        throw WorldCreationException("Error loading the street grid " + path + ": " + e.what());
    }
}

StreetMap load_street_map(const CityLoaderConfig& city_config, CityUtils& city_utils) {

    const auto load_boxes = [&city_utils](const std::string& file) {
        const auto polygons = city_utils.load_polygons_from_file(file);
        std::vector<StreetRect> boxes(polygons.size());
        std::transform(polygons.begin(), polygons.end(), boxes.begin(), box_of);
        return boxes;
    };

    BridgeLayout bridges;
    bridges.bridges = load_boxes(city_config.objects.bridge_file);
    bridges.on_bridge_sensors = load_boxes(city_config.sensors.on_bridge_sensor_file);
    bridges.under_bridge_sensors = load_boxes(city_config.sensors.under_bridge_sensor_file);
    bridges.on_bridge_walls = load_boxes(city_config.collidables.on_bridge_collidable_file);

    return StreetMap{load_streets_grid(city_config.streets_file), std::move(bridges)};
}

std::optional<std::vector<PathStep>> find_street_path(const StreetMap& street_map,
                                                      const StreetPoint from,
                                                      const StreetPoint to) {

    const StreetsGrid& grid = street_map.grid;
    const std::optional<StreetCell> start = grid.nearest_street_cell(from);
    const std::optional<StreetCell> goal = grid.nearest_street_cell(to);

    if (!start || !goal)
        return std::nullopt;

    const int width = grid.get_width();
    CellDescriptions cells(street_map);

    // State: (cell, layer, heading inside a bridge), flattened into one index
    const auto state_of = [width](const int col, const int row, const CarLayer layer,
                                  const int heading) {
        return ((row * width + col) * LAYERS + static_cast<int>(layer)) * HEADINGS + heading;
    };
    const size_t states = cells.size() * LAYERS * HEADINGS;
    std::vector<int32_t> parent(states, -1);
    std::vector<int32_t> queue;
    queue.reserve(cells.size());

    const CellInfo& start_info = cells.at(start->col, start->row);
    for (const CarLayer layer: start_layers(start_info)) {
        const int state = state_of(start->col, start->row, layer, NO_HEADING);
        parent[state] = state;
        queue.push_back(state);
    }

    for (size_t next = 0; next < queue.size(); next++) {

        const int state = queue[next];
        const int heading = state % HEADINGS;
        const auto layer = static_cast<CarLayer>(state / HEADINGS % LAYERS);
        const int cell = state / HEADINGS / LAYERS;
        const int col = cell % width;
        const int row = cell / width;

        if (col == goal->col && row == goal->row) {

            std::vector<PathStep> path;
            for (int step = state;; step = parent[step]) {
                const int step_cell = step / HEADINGS / LAYERS;
                path.push_back(PathStep{step_cell % width, step_cell / width,
                                        static_cast<CarLayer>(step / HEADINGS % LAYERS)});
                if (parent[step] == step)
                    break;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        const CellInfo info = cells.at(col, row);

        for (size_t move = 0; move < MOVES.size(); move++) {

            const int next_col = col + MOVES[move][0];
            const int next_row = row + MOVES[move][1];

            if (!grid.is_street(next_col, next_row))
                continue;

            const CellInfo& next_info = cells.at(next_col, next_row);
            const CarLayer next_layer_value = next_layer(layer, info, next_info);

            // The rail stops a car on the bridge before it leaves the sensor
            if (next_info.rail &&
                (layer == CarLayer::ON_BRIDGE || next_layer_value == CarLayer::ON_BRIDGE))
                continue;

            // Inside a bridge the path keeps the heading it entered with: it never turns from the
            // road on top to the one below (or the other way around)
            const int move_heading = static_cast<int>(move) + 1;
            const bool stays_in_bridge = next_info.bridge != -1 && next_info.bridge == info.bridge;
            if (stays_in_bridge && heading != NO_HEADING && heading != move_heading)
                continue;
            const int next_heading = next_info.bridge == -1 ? NO_HEADING : move_heading;
            const int next_state = state_of(next_col, next_row, next_layer_value, next_heading);
            if (parent[next_state] != -1)
                continue;

            parent[next_state] = state;
            queue.push_back(next_state);
        }
    }

    return std::nullopt;
}

StreetPoint path_midpoint(const StreetsGrid& grid, const std::vector<PathStep>& path) {
    return path_point_at(grid, path, path_length(grid, path) / 2.0f);
}

float path_length(const StreetsGrid& grid, const std::vector<PathStep>& path) {

    if (path.empty())
        return 0.0f;

    // Neighbor cells: every segment between two consecutive centers is one cell long
    return static_cast<float>(path.size() - 1) * static_cast<float>(grid.get_cell_size());
}

StreetPoint path_point_at(const StreetsGrid& grid, const std::vector<PathStep>& path,
                          const float distance) {

    if (path.empty())
        return {0.0f, 0.0f};

    const auto cell_size = static_cast<float>(grid.get_cell_size());
    const float clamped = std::clamp(distance, 0.0f, path_length(grid, path));
    const auto segment = static_cast<size_t>(clamped / cell_size);

    const StreetPoint from = grid.cell_center(path[segment].col, path[segment].row);
    if (segment + 1 >= path.size())
        return from;

    const StreetPoint to = grid.cell_center(path[segment + 1].col, path[segment + 1].row);
    const float fraction = (clamped - static_cast<float>(segment) * cell_size) / cell_size;

    return {from.x + (to.x - from.x) * fraction, from.y + (to.y - from.y) * fraction};
}

std::optional<PathSpot> place_on_path(const StreetMap& street_map,
                                      const std::vector<PathStep>& path, const float half_size) {

    if (path.empty())
        return std::nullopt;

    const float length = path_length(street_map.grid, path);
    const float middle = length / 2.0f;
    const auto max_offset = static_cast<int>(std::ceil(length));

    // 1 px steps away from the middle, ahead (towards Q) before back on a tie
    for (int offset = 0; offset <= max_offset; ++offset) {
        for (const int direction: {1, -1}) {

            const float distance = middle + static_cast<float>(direction * offset);
            if (distance < 0.0f || distance > length || (offset == 0 && direction == -1))
                continue;

            const StreetPoint point = path_point_at(street_map.grid, path, distance);
            const std::optional<bool> inside =
                    inside_a_bridge(street_map.bridges.bridges, point, half_size);
            if (!inside)
                continue;

            const CarLayer layer = *inside ? path_layer_at(street_map.grid, path, distance, point) :
                                             CarLayer::ON_GROUND;
            return PathSpot{point, layer};
        }
    }

    return std::nullopt;
}
