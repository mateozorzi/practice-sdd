#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include "../common/constants.h"
#include "../server/city_utils.h"
#include "../server/world_creation_exception.h"

#include "server_test_utils.h"

namespace {

const std::vector<std::vector<int>> SQUARE = {{0, 0}, {30, 0}, {30, 30}, {0, 30}};

int count_bodies(b2World& world, b2BodyType type) {
    int count = 0;
    for (b2Body* body = world.GetBodyList(); body; body = body->GetNext())
        if (body->GetType() == type)
            count++;
    return count;
}

class CityUtilsTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    CityUtils city_utils{config};
    b2World world{b2Vec2(0.0f, 0.0f)};
    std::filesystem::path temp_file =
            std::filesystem::temp_directory_path() / "nfs_city_utils_test.json";

    void TearDown() override { std::filesystem::remove(temp_file); }
};

}  // namespace


TEST_F(CityUtilsTest, NormalizesCityNames) {
    EXPECT_EQ(city_utils.normalize_city_name("Liberty City"), "liberty_city");
    EXPECT_EQ(city_utils.normalize_city_name("SAN ANDREAS"), "san_andreas");
    EXPECT_EQ(city_utils.normalize_city_name("vice_city"), "vice_city");
}

TEST_F(CityUtilsTest, ReturnsTheConfigOfEachCity) {
    EXPECT_EQ(&city_utils.obtain_city_loader_config("liberty_city"), &config.world.liberty_city);
    EXPECT_EQ(&city_utils.obtain_city_loader_config("san_andreas"), &config.world.san_andreas);
    EXPECT_EQ(&city_utils.obtain_city_loader_config("vice_city"), &config.world.vice_city);
}

TEST_F(CityUtilsTest, UnknownCityThrows) {
    EXPECT_THROW(city_utils.obtain_city_loader_config("gotham"), std::runtime_error);
}

TEST_F(CityUtilsTest, LoadsPolygonsFromRepositoryMap) {
    auto polygons = city_utils.load_polygons_from_file(
            config.world.liberty_city.sensors.on_bridge_sensor_file);

    ASSERT_FALSE(polygons.empty());
    for (const auto& polygon: polygons) {
        ASSERT_EQ(polygon.size(), POLYGONS_SIZE);
        for (const auto& vertex: polygon) EXPECT_EQ(vertex.size(), 2u);
    }
}

TEST_F(CityUtilsTest, MissingPolygonFileThrows) {
    EXPECT_THROW(city_utils.load_polygons_from_file("/does/not/exist.json"),
                 WorldCreationException);
}

TEST_F(CityUtilsTest, PolygonFileMustBeAnArray) {
    std::ofstream(temp_file) << R"({"not": "an array"})";

    EXPECT_THROW(city_utils.load_polygons_from_file(temp_file.string()), WorldCreationException);
}

TEST_F(CityUtilsTest, AddCollidableCreatesStaticBodyWithFilter) {
    city_utils.add_collidable(world, SQUARE, CATEGORY_GROUND, MASK_GROUND);

    ASSERT_EQ(world.GetBodyCount(), 1);
    b2Body* body = world.GetBodyList();
    EXPECT_EQ(body->GetType(), b2_staticBody);
    const b2Fixture* fixture = body->GetFixtureList();
    ASSERT_NE(fixture, nullptr);
    EXPECT_FALSE(fixture->IsSensor());
    EXPECT_EQ(fixture->GetFilterData().categoryBits, CATEGORY_GROUND);
    EXPECT_EQ(fixture->GetFilterData().maskBits, MASK_GROUND);
}

TEST_F(CityUtilsTest, PolygonsMustHaveFourVertices) {
    const std::vector<std::vector<int>> triangle = {{0, 0}, {30, 0}, {30, 30}};

    EXPECT_THROW(city_utils.add_collidable(world, triangle, CATEGORY_GROUND, MASK_GROUND),
                 WorldCreationException);
    EXPECT_THROW(city_utils.add_sensor(world, triangle, SensorType::ON_BRIDGE, CATEGORY_SENSOR,
                                       MASK_SENSOR),
                 WorldCreationException);
}

TEST_F(CityUtilsTest, AddSensorStoresSensorType) {
    city_utils.add_sensor(world, SQUARE, SensorType::UNDER_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);

    b2Fixture* fixture = world.GetBodyList()->GetFixtureList();
    EXPECT_TRUE(fixture->IsSensor());
    EXPECT_EQ(fixture->GetUserData().pointer, static_cast<uintptr_t>(SensorType::UNDER_BRIDGE));
}

TEST_F(CityUtilsTest, AddCheckpointEncodesIdAndTypeInUserData) {
    CheckpointDTO checkpoint(7, 'H', 300, 600, 66, 15);

    city_utils.add_checkpoint(world, checkpoint, CATEGORY_SENSOR, MASK_SENSOR);

    b2Body* body = world.GetBodyList();
    const float pixels_per_meter = static_cast<float>(config.world.pixels_per_meter);
    EXPECT_FLOAT_EQ(body->GetPosition().x, 300.0f / pixels_per_meter);
    EXPECT_FLOAT_EQ(body->GetPosition().y, 600.0f / pixels_per_meter);

    b2Fixture* fixture = body->GetFixtureList();
    EXPECT_TRUE(fixture->IsSensor());
    uintptr_t data = fixture->GetUserData().pointer;
    EXPECT_EQ(static_cast<SensorType>(data & SENSOR_TYPE_MASK), SensorType::CHECKPOINT);
    EXPECT_EQ(data >> SENSOR_DATA_SHIFT, 7u);
}

TEST_F(CityUtilsTest, CleanupOnlyDestroysStaticBodies) {
    city_utils.add_collidable(world, SQUARE, CATEGORY_GROUND, MASK_GROUND);
    city_utils.add_sensor(world, SQUARE, SensorType::ON_BRIDGE, CATEGORY_SENSOR, MASK_SENSOR);
    b2BodyDef dynamic_definition;
    dynamic_definition.type = b2_dynamicBody;
    world.CreateBody(&dynamic_definition);

    city_utils.cleanup(world);

    EXPECT_EQ(count_bodies(world, b2_staticBody), 0);
    EXPECT_EQ(count_bodies(world, b2_dynamicBody), 1);
}
