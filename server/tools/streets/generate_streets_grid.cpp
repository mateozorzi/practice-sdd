// Generates the street grid of a city from its B_STREETS mask (see `make streets`):
//   nfs_streets_generator <mask.png> <streets_<city>.json> <cell_size>

#include <QImage>
#include <QString>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "streets_grid_generator.h"

int main(int argc, const char* const argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <mask.png> <output.json> <cell_size>" << std::endl;
        return 1;
    }

    try {
        const std::string mask_path = argv[1];
        const std::string output_path = argv[2];
        const int cell_size = std::stoi(argv[3]);

        const QImage mask(QString::fromStdString(mask_path));
        if (mask.isNull()) {
            std::cerr << "Could not read the mask " << mask_path << std::endl;
            return 1;
        }

        const std::vector<std::string> rows = classify_street_cells(mask, cell_size);
        if (rows.empty()) {
            std::cerr << "The cell size must be positive" << std::endl;
            return 1;
        }

        write_streets_grid_json(output_path, cell_size, rows);
        std::cout << output_path << ": " << rows.front().size() << " x " << rows.size()
                  << " cells of " << cell_size << " px" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
