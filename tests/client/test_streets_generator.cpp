#include <QColor>
#include <QImage>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "server/tools/streets/streets_grid_generator.h"

TEST(StreetsGeneratorTest, ClassifiesStreetPixels) {
    // 30 x 20 px mask: 3 x 2 cells of 10 px, centers at (0, 0), (10, 0), (20, 0), (0, 10), ...
    QImage mask(30, 20, QImage::Format_RGBA8888);
    mask.fill(QColor(0, 0, 0));
    mask.setPixelColor(0, 0, QColor(255, 36, 250));    // exact street color
    mask.setPixelColor(20, 10, QColor(240, 50, 230));  // street, within the tolerance of 30
    mask.setPixelColor(10, 0, QColor(200, 36, 250));   // too far in red: not a street
    mask.setPixelColor(5, 5, QColor(255, 36, 250));    // street, but not the center of a cell

    const std::vector<std::string> rows = classify_street_cells(mask, 10);

    EXPECT_EQ(rows, (std::vector<std::string>{"100", "001"}));
}
