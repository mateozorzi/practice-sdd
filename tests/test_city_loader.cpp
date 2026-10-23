#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include "../server/city_loader.h"
#include "../server/npc.h"
#include "../server/streets_grid.h"
#include "../server/world_creation_exception.h"

#include "server_test_utils.h"

namespace {

int count_bodies(b2World& world, b2BodyType type) {
    int count = 0;
    for (b2Body* body = world.GetBodyList(); body; body = body->GetNext())
        if (body->GetType() == type)
            count++;
    return count;
}

class NpcTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    b2World world{b2Vec2(0.0f, 0.0f)};

    // Square route, 320 pixels per side: 32 updates of 10 pixels each, exact in floating point
    const std::vector<std::vector<int>> square = {{0, 0}, {320, 0}, {320, 320}, {0, 320}};
};

class CityLoaderTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    b2World world{b2Vec2(0.0f, 0.0f)};
    std::filesystem::path npc_file =
            std::filesystem::temp_directory_path() / "nfs_city_loader_npcs.json";

    void TearDown() override { std::filesystem::remove(npc_file); }
};

}  // namespace


TEST_F(NpcTest, EmptyRouteCreatesNoBody) {
    Npc npc(3, {}, 10.0f, world, config);

    npc.update();

    EXPECT_EQ(npc.get_body(), nullptr);
    EXPECT_EQ(world.GetBodyCount(), 0);
    NpcDTO position = npc.get_position();
    EXPECT_EQ(position.npc_id, 3u);
    EXPECT_EQ(position.x_position, 0u);
    EXPECT_EQ(position.y_position, 0u);
}

TEST_F(NpcTest, SinglePointRouteDoesNotMove) {
    Npc npc(1, {{300, 600}}, 10.0f, world, config);

    npc.update();

    ASSERT_NE(npc.get_body(), nullptr);
    EXPECT_EQ(npc.get_body()->GetType(), b2_kinematicBody);
    EXPECT_NEAR(npc.get_position().x_position, 300, 1);
    EXPECT_NEAR(npc.get_position().y_position, 600, 1);
    EXPECT_EQ(npc.get_position().angle, 0u);
}

TEST_F(NpcTest, InvalidPointsAreSkipped) {
    Npc npc(1, {{5}, {300, 600}}, 10.0f, world, config);

    EXPECT_NEAR(npc.get_position().x_position, 300, 1);
    EXPECT_NEAR(npc.get_position().y_position, 600, 1);
}

TEST_F(NpcTest, StartsAtFirstPointFacingTheSecond) {
    Npc npc(1, square, 10.0f, world, config);

    EXPECT_NEAR(npc.get_position().x_position, 0, 1);
    EXPECT_NEAR(npc.get_position().y_position, 0, 1);
    // The body angle is set on the first update
    npc.update();
    EXPECT_EQ(npc.get_position().angle, 0u);
}

TEST_F(NpcTest, AdvancesAlongTheRoute) {
    Npc npc(1, square, 10.0f, world, config);

    npc.update();
    EXPECT_NEAR(npc.get_position().x_position, 10, 1);
    EXPECT_NEAR(npc.get_position().y_position, 0, 1);

    // 32 updates of 10 pixels reach the first corner and turn down
    for (int i = 0; i < 32; ++i) npc.update();
    EXPECT_NEAR(npc.get_position().x_position, 320, 1);
    EXPECT_NEAR(npc.get_position().y_position, 10, 1);
    EXPECT_NEAR(npc.get_position().angle, 90, 1);
}

TEST_F(NpcTest, LoopsBackToTheStartOfTheRoute) {
    Npc npc(1, square, 10.0f, world, config);

    // Four sides of 320 pixels, 10 pixels per update
    for (int i = 0; i < 128; ++i) npc.update();

    EXPECT_NEAR(npc.get_position().x_position, 0, 1);
    EXPECT_NEAR(npc.get_position().y_position, 0, 1);
}

TEST_F(NpcTest, SkipsRepeatedPoints) {
    Npc npc(1, {{0, 0}, {0, 0}, {300, 0}}, 10.0f, world, config);

    npc.update();
    npc.update();

    EXPECT_GT(npc.get_position().x_position, 0u);
    EXPECT_NEAR(npc.get_position().y_position, 0, 1);
}

TEST_F(NpcTest, FastNpcCanSkipSeveralSegments) {
    Npc npc(1, {{0, 0}, {30, 0}, {30, 30}, {30, 30}}, 70.0f, world, config);

    npc.update();

    // Two segments of 30 pixels plus a zero length one: it ends on the repeated point
    EXPECT_NEAR(npc.get_position().x_position, 30, 1);
    EXPECT_NEAR(npc.get_position().y_position, 30, 1);
}


TEST_F(CityLoaderTest, LoadsEveryCity) {
    RaceDTO race = make_test_race(3, 1);

    for (const std::string city: {"Liberty City", "San Andreas", "Vice City"}) {
        b2World city_world(b2Vec2(0.0f, 0.0f));
        CityLoader loader(city_world, config);

        loader.load_city(city, race.checkpoints);

        EXPECT_GT(count_bodies(city_world, b2_staticBody), 3) << city;
    }
}

TEST_F(CityLoaderTest, CheckpointsAreLoadedAsSensors) {
    CityLoader loader(world, config);
    loader.load_city("Liberty City", {});
    const int without_checkpoints = count_bodies(world, b2_staticBody);

    b2World other_world(b2Vec2(0.0f, 0.0f));
    CityLoader other_loader(other_world, config);
    other_loader.load_city("Liberty City", make_test_race(5, 1).checkpoints);

    EXPECT_EQ(count_bodies(other_world, b2_staticBody), without_checkpoints + 5);
}

TEST_F(CityLoaderTest, ReloadingReplacesThePreviousCity) {
    CityLoader loader(world, config);
    RaceDTO race = make_test_race(3, 1);

    loader.load_city("Liberty City", race.checkpoints);
    const int first_load = count_bodies(world, b2_staticBody);
    loader.load_city("Liberty City", race.checkpoints);

    EXPECT_EQ(count_bodies(world, b2_staticBody), first_load);
}

TEST_F(CityLoaderTest, LoadsNpcsAndMovesThem) {
    std::ofstream(npc_file) << "[[[0,0],[300,0],[300,300]], [[0,0],[10,0]], \"bad\"]";
    config.world.liberty_city.npc_file = npc_file.string();
    CityLoader loader(world, config);

    loader.load_city("Liberty City", {});

    // Routes with less than three points are ignored
    ASSERT_EQ(loader.get_npcs().size(), 1u);
    loader.update_npcs();
    EXPECT_GT(loader.get_npcs()[0].get_position().x_position, 0u);
}

TEST_F(CityLoaderTest, MissingNpcFileLoadsNoNpcs) {
    config.world.liberty_city.npc_file = "/does/not/exist.json";
    CityLoader loader(world, config);

    loader.load_city("Liberty City", {});

    EXPECT_TRUE(loader.get_npcs().empty());
}

TEST_F(CityLoaderTest, NpcFileMustBeAnArray) {
    std::ofstream(npc_file) << R"({"npcs": []})";
    config.world.liberty_city.npc_file = npc_file.string();
    CityLoader loader(world, config);

    loader.load_city("Liberty City", {});

    EXPECT_TRUE(loader.get_npcs().empty());
}

TEST_F(CityLoaderTest, MalformedNpcFileThrows) {
    std::ofstream(npc_file) << "[[[0,0],[1,";
    config.world.liberty_city.npc_file = npc_file.string();
    CityLoader loader(world, config);

    EXPECT_THROW(loader.load_city("Liberty City", {}), WorldCreationException);
}

TEST_F(CityLoaderTest, InvalidPolygonThrows) {
    std::ofstream(npc_file) << "[[[0,0],[30,0],[30,30]]]";
    config.world.liberty_city.collidables.on_ground_collidable_file = npc_file.string();
    CityLoader loader(world, config);

    EXPECT_THROW(loader.load_city("Liberty City", {}), WorldCreationException);
}

TEST_F(CityLoaderTest, InvalidSensorThrows) {
    std::ofstream(npc_file) << "[[[0,0],[30,0],[30,30]]]";
    config.world.liberty_city.sensors.on_bridge_sensor_file = npc_file.string();
    CityLoader loader(world, config);

    EXPECT_THROW(loader.load_city("Liberty City", {}), WorldCreationException);
}

TEST_F(CityLoaderTest, UnknownCityThrows) {
    CityLoader loader(world, config);

    EXPECT_THROW(loader.load_city("Gotham", {}), std::runtime_error);
}

TEST_F(CityLoaderTest, LoadsTheStreetMapOfTheCity) {
    CityLoader city_loader(world, config);
    EXPECT_EQ(city_loader.get_street_map(), nullptr);

    city_loader.load_city("Liberty City", {});

    const StreetMap* street_map = city_loader.get_street_map();
    ASSERT_NE(street_map, nullptr);
    EXPECT_GT(street_map->grid.get_width(), 0);
    EXPECT_FALSE(street_map->bridges.bridges.empty());
}

TEST_F(CityLoaderTest, MissingStreetGridLoadsNoStreetMap) {
    config.world.liberty_city.streets_file = "";
    config.world.san_andreas.streets_file = "/does/not/exist.json";
    CityLoader city_loader(world, config);

    city_loader.load_city("Liberty City", {});
    EXPECT_EQ(city_loader.get_street_map(), nullptr);

    EXPECT_NO_THROW(city_loader.load_city("San Andreas", {}));
    EXPECT_EQ(city_loader.get_street_map(), nullptr);
}
