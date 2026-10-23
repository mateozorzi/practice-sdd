#include "streets_grid_generator.h"

#include <QColor>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace {

// Same street color and tolerance as the editor (Creador::is_street)
constexpr int STREET_RED = 255;
constexpr int STREET_GREEN = 36;
constexpr int STREET_BLUE = 250;
constexpr int STREET_TOLERANCE = 30;

bool is_street_color(const QColor& color) {
    return std::abs(color.red() - STREET_RED) < STREET_TOLERANCE &&
           std::abs(color.green() - STREET_GREEN) < STREET_TOLERANCE &&
           std::abs(color.blue() - STREET_BLUE) < STREET_TOLERANCE;
}

}  // namespace

std::vector<std::string> classify_street_cells(const QImage& mask, const int cell_size) {
    if (mask.isNull() || cell_size <= 0)
        return {};

    // Cell centers at (col * cell_size, row * cell_size), so the last one is still inside the image
    const int width = (mask.width() + cell_size - 1) / cell_size;
    const int height = (mask.height() + cell_size - 1) / cell_size;

    std::vector<std::string> rows(height, std::string(width, '0'));
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            if (is_street_color(mask.pixelColor(col * cell_size, row * cell_size)))
                rows[row][col] = '1';
        }
    }

    return rows;
}

void write_streets_grid_json(const std::string& path, const int cell_size,
                             const std::vector<std::string>& rows) {
    nlohmann::json json;
    json["cell_size"] = cell_size;
    json["width"] = rows.empty() ? 0 : rows.front().size();
    json["height"] = rows.size();
    json["rows"] = rows;

    std::ofstream file(path);
    if (!file)
        throw std::runtime_error("Could not write the street grid " + path);

    // One row per line, so a regenerated grid has a readable diff
    file << "{\n  \"cell_size\": " << json["cell_size"] << ",\n  \"width\": " << json["width"]
         << ",\n  \"height\": " << json["height"] << ",\n  \"rows\": [\n";
    for (size_t row = 0; row < rows.size(); ++row)
        file << "    " << json["rows"][row] << (row + 1 < rows.size() ? ",\n" : "\n");
    file << "  ]\n}\n";

    if (!file)
        throw std::runtime_error("Could not write the street grid " + path);
}
