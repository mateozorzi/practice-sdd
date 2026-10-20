#include <string>
#include <unordered_map>
#include <vector>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include "../common/constants.h"
#include "../server/city_utils.h"
#include "../server/collision_listener.h"

#include "server_test_utils.h"

namespace {

class CollisionListenerTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    b2World world{b2Vec2(0.0f, 0.0f)};
    std::unordered_map<uint32_t, Player> players;
    CollisionListener listener{players};
    CityUtils city_utils{config};

    void SetUp() override { world.SetContactListener(&listener); }

    Player& add_player(uint32_t id, float x, float y) {
        auto [it, _] = players.try_emplace(id, id, "player" + std::to_string(id));
        it->second.create_car("Compact", config, world);
        it->second.get_car_body()->SetTransform(b2Vec2(x, y), 0.0f);
        return it->second;
    }

    // Square polygon in pixels centered in (x, y) meters
    std::vector<std::vector<int>> square_at(float x, float y, int half_side_pixels) const {
        const int cx = static_cast<int>(x * config.world.pixels_per_meter);
        const int cy = static_cast<int>(y * config.world.pixels_per_meter);
        return {{cx - half_side_pixels, cy - half_side_pixels},
                {cx + half_side_pixels, cy - half_side_pixels},
                {cx + half_side_pixels, cy + half_side_pixels},
                {cx - half_side_pixels, cy + half_side_pixels}};
    }

    void step(int times = 1) {
        for (int i = 0; i < times; ++i)
            world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    }
};

}  // namespace


TEST_F(CollisionListenerTest, CrashingIntoAWallDamagesTheCar) {
    const Player& player = add_player(1, 0.0f, 0.0f);
    const uint16_t base = player.get_health();
    city_utils.add_collidable(world, square_at(5.0f, 0.0f, 30), CATEGORY_GROUND, MASK_GROUND);

    player.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(60);

    EXPECT_LT(player.get_health(), base);
    EXPECT_GE(player.get_health(), base - static_cast<uint16_t>(STATIC_COLLISION_DAMAGE_MAX));
}

TEST_F(CollisionListenerTest, SlowContactDoesNoDamage) {
    const Player& player = add_player(1, 0.0f, 0.0f);
    const uint16_t base = player.get_health();
    city_utils.add_collidable(world, square_at(3.0f, 0.0f, 30), CATEGORY_GROUND, MASK_GROUND);

    for (int i = 0; i < 120; ++i) {
        player.get_car_body()->SetLinearVelocity(b2Vec2(0.5f, 0.0f));
        step();
    }

    EXPECT_EQ(player.get_health(), base);
}

TEST_F(CollisionListenerTest, CarOnBridgeIgnoresGroundWalls) {
    Player& player = add_player(1, 0.0f, 0.0f);
    const uint16_t base = player.get_health();
    player.set_car_layer(CarLayer::ON_BRIDGE);
    city_utils.add_collidable(world, square_at(5.0f, 0.0f, 30), CATEGORY_GROUND, MASK_GROUND);

    player.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(60);

    EXPECT_EQ(player.get_health(), base);
    EXPECT_GT(player.get_car_body()->GetPosition().x, 5.0f);
}

TEST_F(CollisionListenerTest, TwoCarsCrashingDamageEachOther) {
    const Player& first = add_player(1, -5.0f, 0.0f);
    const Player& second = add_player(2, 5.0f, 0.0f);
    const uint16_t base = first.get_health();

    first.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    second.get_car_body()->SetLinearVelocity(b2Vec2(-10.0f, 0.0f));
    step(60);

    EXPECT_LT(first.get_health(), base);
    EXPECT_LT(second.get_health(), base);
}

TEST_F(CollisionListenerTest, BridgeSensorsChangeTheCarLayer) {
    const Player& player = add_player(1, 0.0f, 0.0f);
    city_utils.add_sensor(world, square_at(0.0f, 0.0f, 90), SensorType::ON_BRIDGE, CATEGORY_SENSOR,
                          MASK_SENSOR);

    step();
    EXPECT_EQ(player.get_car_layer(), CarLayer::ON_BRIDGE);

    player.get_car_body()->SetTransform(b2Vec2(50.0f, 50.0f), 0.0f);
    step();
    EXPECT_EQ(player.get_car_layer(), CarLayer::ON_GROUND);
}

TEST_F(CollisionListenerTest, UnderBridgeSensorsChangeTheCarLayer) {
    const Player& player = add_player(1, 0.0f, 0.0f);
    city_utils.add_sensor(world, square_at(0.0f, 0.0f, 90), SensorType::UNDER_BRIDGE,
                          CATEGORY_SENSOR, MASK_SENSOR);

    step();
    EXPECT_EQ(player.get_car_layer(), CarLayer::UNDER_BRIDGE);

    player.get_car_body()->SetTransform(b2Vec2(50.0f, 50.0f), 0.0f);
    step();
    EXPECT_EQ(player.get_car_layer(), CarLayer::ON_GROUND);
}

TEST_F(CollisionListenerTest, LeavingAnotherSensorKeepsTheLayer) {
    Player& player = add_player(1, 0.0f, 0.0f);
    player.set_car_layer(CarLayer::ON_BRIDGE);
    city_utils.add_sensor(world, square_at(0.0f, 0.0f, 90), SensorType::UNDER_BRIDGE,
                          CATEGORY_SENSOR, MASK_SENSOR);

    step();
    EXPECT_EQ(player.get_car_layer(), CarLayer::ON_BRIDGE);

    player.get_car_body()->SetTransform(b2Vec2(50.0f, 50.0f), 0.0f);
    step();
    EXPECT_EQ(player.get_car_layer(), CarLayer::ON_BRIDGE);
}

TEST_F(CollisionListenerTest, OnlyTheNextCheckpointIsTriggered) {
    RaceDTO race = make_test_race(3, 1);
    Player& player = add_player(1, 0.0f, 0.0f);
    player.set_position_for_race(race.checkpoints);
    player.start_race();
    const float ppm = static_cast<float>(config.world.pixels_per_meter);

    // Checkpoint 2 is not the next one: the car goes through it without effect
    CheckpointDTO second = race.checkpoints[1];
    player.get_car_body()->SetTransform(b2Vec2(second.x / ppm, second.y / ppm), 0.0f);
    city_utils.add_checkpoint(world, second, CATEGORY_SENSOR, MASK_SENSOR);
    step();
    player.update_car_position(race, 0, config.game);
    EXPECT_TRUE(player.is_next_checkpoint(1));

    CheckpointDTO first = race.checkpoints[0];
    player.get_car_body()->SetTransform(b2Vec2(first.x / ppm, first.y / ppm), 0.0f);
    city_utils.add_checkpoint(world, first, CATEGORY_SENSOR, MASK_SENSOR);
    step();
    player.update_car_position(race, 0, config.game);
    EXPECT_TRUE(player.is_next_checkpoint(2));
}

TEST_F(CollisionListenerTest, BodiesWithoutPlayerAreIgnored) {
    b2BodyDef definition;
    definition.type = b2_dynamicBody;
    b2Body* body = world.CreateBody(&definition);
    b2PolygonShape shape;
    shape.SetAsBox(1.0f, 1.0f);
    b2Fixture* fixture = body->CreateFixture(&shape, 1.0f);
    b2Filter filter;
    filter.categoryBits = CATEGORY_CAR_GROUND;
    filter.maskBits = MASK_CAR_GROUND;
    fixture->SetFilterData(filter);
    city_utils.add_sensor(world, square_at(0.0f, 0.0f, 90), SensorType::ON_BRIDGE, CATEGORY_SENSOR,
                          MASK_SENSOR);
    city_utils.add_collidable(world, square_at(5.0f, 0.0f, 30), CATEGORY_GROUND, MASK_GROUND);

    body->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(60);

    SUCCEED();
}

TEST_F(CollisionListenerTest, NullContactsAreIgnored) {
    listener.BeginContact(nullptr);
    listener.EndContact(nullptr);
    listener.PreSolve(nullptr, nullptr);

    SUCCEED();
}
