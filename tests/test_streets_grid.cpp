#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../server/server_config_loader.h"
#include "../server/streets_grid.h"
#include "../server/world_creation_exception.h"

#include "server_test_utils.h"
#include "streets_test_utils.h"

namespace {

// Temporary folder for the grid files written by each test
class StreetsGridTest: public ::testing::Test {
protected:
    std::filesystem::path folder;

    void SetUp() override {
        const auto* test = ::testing::UnitTest::GetInstance()->current_test_info();
        folder = std::filesystem::temp_directory_path() /
                 (std::string("nfs_streets_grid_") + test->name());
        std::filesystem::create_directories(folder);
    }

    void TearDown() override { std::filesystem::remove_all(folder); }

    std::string grid_path() const { return (folder / "streets_test.json").string(); }

    std::string write_grid(const std::string& contents) const {
        std::ofstream(grid_path()) << contents;
        return grid_path();
    }

    // Message of the error thrown when loading `contents`, empty if it does not throw
    std::string load_error(const std::string& contents) const {
        const std::string path = write_grid(contents);
        try {
            load_streets_grid(path);
        } catch (const WorldCreationException& error) {
            return error.what();
        }
        return "";
    }
};

float path_length(const std::vector<PathStep>& path) {
    return static_cast<float>((path.size() - 1) * TEST_CELL_SIZE);
}

bool contains_cell(const std::vector<PathStep>& path, const int x, const int y) {
    return std::any_of(path.begin(), path.end(), [x, y](const PathStep& step) {
        return step.col * TEST_CELL_SIZE == x && step.row * TEST_CELL_SIZE == y;
    });
}

// Crossing of the Background: bridge rectangle (100..160, 100..160)
bool inside_bridge(const PathStep& step) {
    const int x = step.col * TEST_CELL_SIZE;
    const int y = step.row * TEST_CELL_SIZE;
    return x >= 100 && x <= 160 && y >= 100 && y <= 160;
}

// True if the path changes its direction between two cells inside the bridge rectangle
bool turns_inside_bridge(const std::vector<PathStep>& path) {
    for (size_t i = 1; i + 1 < path.size(); ++i) {
        if (!inside_bridge(path[i]) || !inside_bridge(path[i + 1]))
            continue;
        const int in_col = path[i].col - path[i - 1].col;
        const int in_row = path[i].row - path[i - 1].row;
        const int out_col = path[i + 1].col - path[i].col;
        const int out_row = path[i + 1].row - path[i].row;
        if (in_col != out_col || in_row != out_row)
            return true;
    }
    return false;
}

const std::vector<std::string> CITIES = {"liberty_city", "san_andreas", "vice_city"};

const CityLoaderConfig& city_config(const ServerConfig& config, const std::string& city) {
    if (city == "liberty_city")
        return config.world.liberty_city;
    if (city == "san_andreas")
        return config.world.san_andreas;
    return config.world.vice_city;
}

}  // namespace


TEST_F(StreetsGridTest, LoadsCellSizeAndStreetCells) {
    const std::string path =
            write_grid(R"({"cell_size": 10, "width": 3, "height": 2, "rows": ["101", "011"]})");

    const StreetsGrid grid = load_streets_grid(path);

    EXPECT_EQ(grid.get_cell_size(), 10);
    EXPECT_EQ(grid.get_width(), 3);
    EXPECT_EQ(grid.get_height(), 2);
    EXPECT_TRUE(grid.is_street(0, 0));
    EXPECT_FALSE(grid.is_street(1, 0));
    EXPECT_TRUE(grid.is_street(2, 0));
    EXPECT_FALSE(grid.is_street(0, 1));
    EXPECT_TRUE(grid.is_street(1, 1));
    EXPECT_TRUE(grid.is_street(2, 1));
}

TEST_F(StreetsGridTest, MalformedJsonThrows) {
    const std::string error = load_error(R"({"cell_size": 10, "width": )");

    EXPECT_NE(error.find(grid_path()), std::string::npos) << error;
}

TEST_F(StreetsGridTest, HeightMismatchThrows) {
    const std::string error =
            load_error(R"({"cell_size": 10, "width": 3, "height": 3, "rows": ["101", "011"]})");

    EXPECT_NE(error.find("height"), std::string::npos) << error;
}

TEST_F(StreetsGridTest, WidthMismatchThrows) {
    const std::string error =
            load_error(R"({"cell_size": 10, "width": 3, "height": 2, "rows": ["101", "01"]})");

    EXPECT_NE(error.find("width"), std::string::npos) << error;
}

TEST(StreetsGridNearestCellTest, PointOutsideTheGridGoesToTheClosestStreetCell) {
    const StreetMap streets = make_two_streets_map();

    const auto far_away = streets.grid.nearest_street_cell({1000, 1000});
    const auto above = streets.grid.nearest_street_cell({-30, -40});
    const auto between = streets.grid.nearest_street_cell({50, 60});

    EXPECT_EQ(far_away, (StreetCell{20, 10}));  // (200, 100)
    EXPECT_EQ(above, (StreetCell{0, 0}));       // (0, 0)
    EXPECT_EQ(between, (StreetCell{5, 10}));    // (50, 100): 40 px away, (50, 0) is 60 px away
    EXPECT_FALSE(StreetsGrid(10, {"000", "000"}).nearest_street_cell({0, 0}).has_value());
}


TEST(StreetPathTest, OverBridgeGoesStraightOnTop) {
    const StreetMap crossing = make_test_crossing_map();

    const auto path = find_street_path(crossing, {0, 130}, {260, 130});

    ASSERT_TRUE(path.has_value());
    EXPECT_FLOAT_EQ(path_length(*path), 260.0f);
    for (const PathStep& step: *path) {
        const int x = step.col * TEST_CELL_SIZE;
        const int y = step.row * TEST_CELL_SIZE;
        const bool bottom_road_outside_bridge = x >= 110 && x <= 150 && (y < 100 || y > 160);
        EXPECT_FALSE(bottom_road_outside_bridge) << "(" << x << ", " << y << ")";
        if (inside_bridge(step)) {
            EXPECT_EQ(step.layer, CarLayer::ON_BRIDGE) << "(" << x << ", " << y << ")";
        }
    }
}

TEST(StreetPathTest, UnderBridgeCrossesBelowTheRails) {
    const StreetMap crossing = make_test_crossing_map();

    const auto path = find_street_path(crossing, {130, 0}, {130, 260});

    ASSERT_TRUE(path.has_value());
    EXPECT_FLOAT_EQ(path_length(*path), 260.0f);
    EXPECT_TRUE(contains_cell(*path, 130, 100));
    EXPECT_TRUE(contains_cell(*path, 130, 160));
    for (const PathStep& step: *path) {
        if (inside_bridge(step)) {
            EXPECT_EQ(step.layer, CarLayer::UNDER_BRIDGE)
                    << "(" << step.col * TEST_CELL_SIZE << ", " << step.row * TEST_CELL_SIZE << ")";
        }
    }
}

TEST(StreetPathTest, FromTopToBottomTakesTheDetour) {
    const StreetMap crossing = make_test_crossing_map();

    const auto path = find_street_path(crossing, {0, 130}, {130, 0});

    ASSERT_TRUE(path.has_value());
    EXPECT_TRUE(contains_cell(*path, 260, 260));
    EXPECT_FLOAT_EQ(path_length(*path), 780.0f);
    EXPECT_FALSE(turns_inside_bridge(*path));
}

TEST(StreetPathTest, FromBottomToTopTakesTheDetour) {
    const StreetMap crossing = make_test_crossing_map();

    const auto path = find_street_path(crossing, {130, 0}, {0, 130});

    ASSERT_TRUE(path.has_value());
    EXPECT_TRUE(contains_cell(*path, 260, 260));
    EXPECT_FLOAT_EQ(path_length(*path), 780.0f);
    EXPECT_FALSE(turns_inside_bridge(*path));
}

TEST(StreetPathTest, StartsFromNearestStreetCell) {
    const StreetMap street = make_straight_street_map();

    const auto path = find_street_path(street, {50, 40}, {200, 0});

    ASSERT_TRUE(path.has_value());
    ASSERT_FALSE(path->empty());
    EXPECT_EQ(path->front().col * TEST_CELL_SIZE, 50);
    EXPECT_EQ(path->front().row * TEST_CELL_SIZE, 0);
    EXPECT_FLOAT_EQ(path_length(*path), 150.0f);
    const StreetPoint midpoint = path_midpoint(street.grid, *path);
    EXPECT_FLOAT_EQ(midpoint.x, 125.0f);
    EXPECT_FLOAT_EQ(midpoint.y, 0.0f);
}


// Documented exception to the Background rule: it checks how the repository configuration is wired,
// not its game values
TEST(StreetsGridInstallTest, ConfigPointsToTheCityGrids) {
    const ServerConfig config = ServerConfigLoader().load_yaml(source_path("server/config"));

    for (const std::string& city: CITIES) {
        const std::string file = "streets_" + city + ".json";
        EXPECT_EQ(city_config(config, city).streets_file, "/var/nfs/world/streets/" + file);
        EXPECT_TRUE(std::filesystem::exists(source_path("server/world/streets/" + file))) << file;
    }
}

TEST(StreetsGridInstallTest, GridsAreInstalled) {
    if (!std::filesystem::exists("/var/nfs/world"))
        GTEST_SKIP() << "the server world files are not installed in /var/nfs/world";

    for (const std::string& city: CITIES) {
        const std::string file = "/var/nfs/world/streets/streets_" + city + ".json";
        EXPECT_TRUE(std::filesystem::exists(file)) << file;
    }
}
