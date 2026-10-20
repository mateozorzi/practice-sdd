#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../server/race_loader.h"

#include "server_test_utils.h"

namespace {

const RaceDTO* find_race(const std::vector<RaceDTO>& races, const std::string& name) {
    auto it = std::find_if(races.begin(), races.end(),
                           [&name](const RaceDTO& race) { return race.name == name; });
    return it == races.end() ? nullptr : &(*it);
}

class RaceLoaderTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    std::filesystem::path races_dir;

    void SetUp() override {
        races_dir = std::filesystem::temp_directory_path() /
                    ("nfs_races_test_" +
                     std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::filesystem::create_directories(races_dir);
    }

    void TearDown() override { std::filesystem::remove_all(races_dir); }

    void write_race(const std::string& file_name, const std::string& city, int laps,
                    const std::vector<std::string>& checkpoint_rows) {
        std::ofstream file(races_dir / file_name);
        file << "background.png,mask.png\n";
        file << "City,n_laps\n";
        file << city << "," << laps << "\n";
        file << "id, orientation, xCenter, yCenter, width, height\n";
        for (const auto& row: checkpoint_rows) file << row << "\n";
    }
};

}  // namespace


TEST_F(RaceLoaderTest, LoadsEveryRaceInTheRepository) {
    std::vector<RaceDTO> races = RaceLoader(config).load_races();

    ASSERT_EQ(races.size(), 6u);
    for (const auto& name: {"bridge_tour", "dock_adventure", "highways_are_not_playgrounds",
                            "ocean_bridge_cruise", "ocean_dream", "park_day"})
        EXPECT_NE(find_race(races, name), nullptr) << "missing race " << name;

    for (const auto& race: races) {
        EXPECT_GT(race.laps, 0) << race.name;
        EXPECT_GE(race.size_checkpoints, 2) << race.name;
        EXPECT_EQ(race.size_checkpoints, race.checkpoints.size()) << race.name;
    }
}

TEST_F(RaceLoaderTest, ParsesRaceMetadataAndCheckpoints) {
    std::vector<RaceDTO> races = RaceLoader(config).load_races();
    const RaceDTO* park_day = find_race(races, "park_day");
    ASSERT_NE(park_day, nullptr);

    EXPECT_EQ(park_day->city, "Liberty City");
    EXPECT_EQ(park_day->laps, 4);

    const CheckpointDTO& first = park_day->checkpoints.front();
    EXPECT_EQ(first.id, 1);
    EXPECT_EQ(first.orientation, 'V');
    EXPECT_EQ(first.x, 1712);
    EXPECT_EQ(first.y, 2391);
    EXPECT_EQ(first.width, 15);
    EXPECT_EQ(first.height, 66);

    for (size_t i = 0; i < park_day->checkpoints.size(); ++i)
        EXPECT_EQ(park_day->checkpoints[i].id, i + 1) << "checkpoints must be ordered";
}

TEST_F(RaceLoaderTest, AttachesCityRenderingData) {
    std::vector<RaceDTO> races = RaceLoader(config).load_races();
    const RaceDTO* park_day = find_race(races, "park_day");
    ASSERT_NE(park_day, nullptr);

    EXPECT_NE(park_day->bridges_data, "[]");
    EXPECT_EQ(park_day->bridges_data.front(), '[');
}

TEST_F(RaceLoaderTest, MissingFolderReturnsNoRaces) {
    config.world.races_folder = (races_dir / "does_not_exist").string();

    EXPECT_TRUE(RaceLoader(config).load_races().empty());
}

TEST_F(RaceLoaderTest, IgnoresFilesThatAreNotCsv) {
    write_race("my_race.csv", "Vice City", 2, {"1,H,100,200,66,15", "2,V,300,400,15,66"});
    std::ofstream(races_dir / "notes.txt") << "not a race";
    config.world.races_folder = races_dir.string();

    std::vector<RaceDTO> races = RaceLoader(config).load_races();

    ASSERT_EQ(races.size(), 1u);
    EXPECT_EQ(races[0].name, "my_race");
    EXPECT_EQ(races[0].city, "Vice City");
    EXPECT_EQ(races[0].laps, 2);
    ASSERT_EQ(races[0].checkpoints.size(), 2u);
    EXPECT_EQ(races[0].checkpoints[1].orientation, 'V');
    EXPECT_EQ(races[0].checkpoints[1].x, 300);
}

TEST_F(RaceLoaderTest, MissingRenderingFilesFallBackToEmptyJson) {
    write_race("my_race.csv", "Vice City", 1, {"1,H,100,200,66,15", "2,H,100,300,66,15"});
    config.world.races_folder = races_dir.string();
    config.world.vice_city.objects.bridge_file = (races_dir / "missing.json").string();
    config.world.vice_city.objects.decoration_file = (races_dir / "missing.json").string();

    std::vector<RaceDTO> races = RaceLoader(config).load_races();

    ASSERT_EQ(races.size(), 1u);
    EXPECT_EQ(races[0].bridges_data, "[]");
    EXPECT_EQ(races[0].decorations_data, "[]");
}

TEST_F(RaceLoaderTest, UnknownCityThrows) {
    write_race("my_race.csv", "Gotham", 1, {"1,H,100,200,66,15", "2,H,100,300,66,15"});
    config.world.races_folder = races_dir.string();

    EXPECT_THROW(RaceLoader(config).load_races(), std::runtime_error);
}
