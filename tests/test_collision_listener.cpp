#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include "../common/constants.h"
#include "../common/power_up_type.h"
#include "../server/city_utils.h"
#include "../server/collision_listener.h"
#include "../server/power_up.h"

#include "server_test_utils.h"
#include "streets_test_utils.h"

namespace {

class CollisionListenerTest: public ::testing::Test {
protected:
    ServerConfig config = make_power_up_test_server_config();
    b2World world{b2Vec2(0.0f, 0.0f)};
    std::unordered_map<uint32_t, Player> players;
    // Milestone 2 Background: fake generator sequence
    PowerUpManager power_ups{
            world, config,
            make_sequence_power_up_type_generator({PowerUpType::Shield, PowerUpType::Nitro,
                                                   PowerUpType::Repair, PowerUpType::Nitro})};
    CollisionListener listener{players, power_ups};
    CityUtils city_utils{config};
    const StreetMap crossing = make_test_crossing_map();

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
        for (int i = 0; i < times; ++i) {
            world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
            power_ups.advance(GAME_STEP);
        }
    }

    // Loads a single power-up centered in (x, y) meters: the midpoint of two checkpoints there
    const PowerUp& place_power_up(float x, float y) {
        const auto px = static_cast<uint16_t>(x * config.world.pixels_per_meter);
        const auto py = static_cast<uint16_t>(y * config.world.pixels_per_meter);
        power_ups.load({CheckpointDTO(1, 'V', px, py), CheckpointDTO(2, 'V', px, py)});
        if (power_ups.get_power_ups().empty())
            throw std::runtime_error("no power-up was loaded");
        return power_ups.get_power_ups().front();
    }

    // Milestone 6: loads the power-up of the stretch P -> Q (pixels) of the crossing of the
    // milestone 5 Background, so it gets the layer of the path
    const PowerUp& place_bridge_power_up(uint16_t px, uint16_t py, uint16_t qx, uint16_t qy) {
        power_ups.load({CheckpointDTO(1, 'V', px, py), CheckpointDTO(2, 'V', qx, qy)}, &crossing);
        if (power_ups.get_power_ups().empty())
            throw std::runtime_error("no power-up was loaded");
        return power_ups.get_power_ups().front();
    }

    float meters(float pixels) const {
        return pixels / static_cast<float>(config.world.pixels_per_meter);
    }

    // Car at (x, y) pixels with the given layer, moving at (vx, vy) meters per second
    Player& add_moving_player(uint32_t id, float x, float y, CarLayer layer, float vx, float vy) {
        Player& player = add_player(id, meters(x), meters(y));
        player.set_car_layer(layer);
        player.get_car_body()->SetLinearVelocity(b2Vec2(vx, vy));
        return player;
    }

    // Box2D sensor over a box in pixels
    void add_bridge_sensor(const StreetRect& box, SensorType type) {
        const auto min_x = static_cast<int>(box.min_x);
        const auto max_x = static_cast<int>(box.max_x);
        const auto min_y = static_cast<int>(box.min_y);
        const auto max_y = static_cast<int>(box.max_y);
        city_utils.add_sensor(world,
                              {{min_x, min_y}, {max_x, min_y}, {max_x, max_y}, {min_x, max_y}},
                              type, CATEGORY_SENSOR, MASK_SENSOR);
    }

    static bool has_any_effect(const Player& player) {
        return player.has_effect(PowerUpType::Nitro) || player.has_effect(PowerUpType::Shield);
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

TEST_F(CollisionListenerTest, CrossingAPowerUpKeepsVelocityAndHealth) {
    const Player& player = add_player(1, 0.0f, 10.0f);
    place_power_up(5.0f, 10.0f);
    b2Body* body = player.get_car_body();
    // Without damping, any change in velocity can only come from the contact
    body->SetLinearDamping(0.0f);
    body->SetLinearVelocity(b2Vec2(10.0f, 0.0f));

    step(60);

    EXPECT_GT(body->GetPosition().x, 6.0f);
    EXPECT_FLOAT_EQ(body->GetLinearVelocity().x, 10.0f);
    EXPECT_FLOAT_EQ(body->GetLinearVelocity().y, 0.0f);
    EXPECT_EQ(player.get_health(), 100);
}

TEST_F(CollisionListenerTest, CarCollectsShieldPowerUp) {
    const Player& player = add_player(1, 0.0f, 10.0f);
    const PowerUp& power_up = place_power_up(5.0f, 10.0f);
    ASSERT_EQ(power_up.get_type(), PowerUpType::Shield);

    player.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(60);

    EXPECT_TRUE(player.has_effect(PowerUpType::Shield));
    EXPECT_FALSE(power_up.is_active());
}

TEST_F(CollisionListenerTest, InactivePowerUpGivesNoEffect) {
    Player& first = add_player(1, 50.0f, 50.0f);
    const PowerUp& power_up = place_power_up(5.0f, 10.0f);
    ASSERT_TRUE(power_ups.try_collect(0, first));
    const Player& second = add_player(2, 0.0f, 10.0f);

    second.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(60);

    EXPECT_FALSE(has_any_effect(second));
    EXPECT_FALSE(power_up.is_active());
}

TEST_F(CollisionListenerTest, TwoCarsSameStepOnlyOneCollects) {
    const PowerUp& power_up = place_power_up(5.0f, 10.0f);
    // Both cars overlap the sensor (but not each other), so both contacts begin in the same step
    const Player& first = add_player(1, 5.0f, 9.6f);
    const Player& second = add_player(2, 5.0f, 10.4f);

    step();

    EXPECT_NE(first.has_effect(PowerUpType::Shield), second.has_effect(PowerUpType::Shield));
    EXPECT_FALSE(power_up.is_active());
}

TEST_F(CollisionListenerTest, DeadCarDoesNotCollect) {
    Player& player = add_player(1, 0.0f, 10.0f);
    player.apply_collision_damage(100);
    ASSERT_TRUE(player.is_dead());
    const PowerUp& power_up = place_power_up(5.0f, 10.0f);

    player.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(60);

    EXPECT_TRUE(power_up.is_active());
    EXPECT_FALSE(has_any_effect(player));
}


// Power-ups by layer on the bridges (scenarios 6.6 - 6.10). The cars of 6.6 - 6.9 keep the layer
// the test sets, because there are no bridge sensors in the world.

TEST_F(CollisionListenerTest, UnderBridgeCarSkipsOnBridgePowerUp) {
    const PowerUp& power_up = place_bridge_power_up(0, 130, 260, 130);
    ASSERT_EQ(power_up.get_layer(), CarLayer::ON_BRIDGE);
    const Player& player = add_moving_player(1, 130, 70, CarLayer::UNDER_BRIDGE, 0.0f, 5.0f);

    step(60);

    ASSERT_GT(player.get_car_body()->GetPosition().y, meters(160));
    EXPECT_TRUE(power_up.is_active());
    EXPECT_FALSE(has_any_effect(player));
    EXPECT_EQ(player.get_health(), 100);
}

TEST_F(CollisionListenerTest, OnBridgeCarCollectsOnBridgePowerUp) {
    const PowerUp& power_up = place_bridge_power_up(0, 130, 260, 130);
    ASSERT_EQ(power_up.get_layer(), CarLayer::ON_BRIDGE);
    ASSERT_EQ(power_up.get_type(), PowerUpType::Shield);
    const Player& player = add_moving_player(1, 70, 130, CarLayer::ON_BRIDGE, 5.0f, 0.0f);

    step(60);

    EXPECT_TRUE(player.has_effect(PowerUpType::Shield));
    EXPECT_FALSE(power_up.is_active());
}

TEST_F(CollisionListenerTest, UnderBridgePowerUpOnlyForUnderBridgeCars) {
    const PowerUp& power_up = place_bridge_power_up(130, 0, 130, 260);
    ASSERT_EQ(power_up.get_layer(), CarLayer::UNDER_BRIDGE);
    const Player& on_bridge = add_moving_player(1, 70, 130, CarLayer::ON_BRIDGE, 5.0f, 0.0f);

    step(60);

    ASSERT_GT(on_bridge.get_car_body()->GetPosition().x, meters(160));
    EXPECT_TRUE(power_up.is_active());
    EXPECT_FALSE(has_any_effect(on_bridge));

    const Player& under_bridge = add_moving_player(2, 130, 70, CarLayer::UNDER_BRIDGE, 0.0f, 5.0f);

    step(60);

    EXPECT_TRUE(under_bridge.has_effect(PowerUpType::Shield));
    EXPECT_FALSE(power_up.is_active());
}

TEST_F(CollisionListenerTest, OnGroundPowerUpForAnyLayer) {
    const PowerUp& power_up = place_power_up(5.0f, 10.0f);
    ASSERT_EQ(power_up.get_layer(), CarLayer::ON_GROUND);
    Player& player = add_player(1, 0.0f, 10.0f);
    player.set_car_layer(CarLayer::ON_BRIDGE);  // for example, on the ramp of a bridge

    player.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(60);

    EXPECT_TRUE(player.has_effect(PowerUpType::Shield));
    EXPECT_FALSE(power_up.is_active());
}

TEST_F(CollisionListenerTest, RealSensorsOnlyTheCarOnTopCollects) {
    add_bridge_sensor(crossing.bridges.on_bridge_sensors.front(), SensorType::ON_BRIDGE);
    add_bridge_sensor(crossing.bridges.under_bridge_sensors.front(), SensorType::UNDER_BRIDGE);
    const PowerUp& power_up = place_bridge_power_up(0, 130, 260, 130);
    ASSERT_EQ(power_up.get_layer(), CarLayer::ON_BRIDGE);
    // Both start on the ground, outside the sensors; the second one waits on the top road
    const Player& below = add_moving_player(1, 130, 0, CarLayer::ON_GROUND, 0.0f, 10.0f);
    const Player& above = add_moving_player(2, 0, 130, CarLayer::ON_GROUND, 0.0f, 0.0f);

    step(90);

    ASSERT_GT(below.get_car_body()->GetPosition().y, meters(260));
    EXPECT_TRUE(power_up.is_active());
    EXPECT_FALSE(has_any_effect(below));

    above.get_car_body()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    step(90);

    ASSERT_GT(above.get_car_body()->GetPosition().x, meters(260));
    EXPECT_TRUE(above.has_effect(PowerUpType::Shield));
    EXPECT_FALSE(power_up.is_active());
}
