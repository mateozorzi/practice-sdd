#ifndef STREETS_GRID_GENERATOR_H
#define STREETS_GRID_GENERATOR_H

// Builds the street grid of a city from its B_STREETS mask. Runs once, outside the game: the server
// only reads the JSON it writes (server/streets_grid.h). It does not include server headers.

#include <QImage>
#include <string>
#include <vector>

// One string per row of cells, '1' when the mask pixel at the cell center (col * cell_size,
// row * cell_size) has the street color (255, 36, 250) within 30 per channel, like the editor
std::vector<std::string> classify_street_cells(const QImage& mask, int cell_size);

// Writes {"cell_size", "width", "height", "rows"}. Throws std::runtime_error if it cannot write.
void write_streets_grid_json(const std::string& path, int cell_size,
                             const std::vector<std::string>& rows);

#endif
