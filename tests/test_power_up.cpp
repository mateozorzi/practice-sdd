#include <set>
#include <vector>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include "../common/power_up_type.h"
#include "../server/player.h"
#include "../server/power_up.h"
#include "../server/server_constants.h"

#include "server_test_utils.h"

namespace {

// Milestone 2 Background: fake generator sequence
const std::vector<PowerUpType> BACKGROUND_SEQUENCE = {PowerUpType::Shield, PowerUpType::Nitro,
                                                      PowerUpType::Repair, PowerUpType::Nitro};

class PowerUpManagerTest: public ::testing::Test {
protected:
    // Declared before the world, the player and the manager, which keep references to it
    ServerConfig config = make_power_up_test_server_config();
    b2World world{b2Vec2(0.0f, 0.0f)};
    Player player{1, "player1"};
    PowerUpManager power_ups{world, config,
                             make_sequence_power_up_type_generator(BACKGROUND_SEQUENCE)};

    void SetUp() override { player.create_car("Compact", config, world); }

    // Advances physics and the respawn cooldown by `times` fixed steps of GAME_STEP
    void step(int times) {
        for (int i = 0; i < times; ++i) {
            world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
            power_ups.advance(GAME_STEP);
        }
    }
};

}  // namespace


TEST_F(PowerUpManagerTest, FourCheckpointsSpawnTwoPowerUps) {
    power_ups.load(make_test_race(4, 1).checkpoints);

    EXPECT_EQ(power_ups.get_power_ups().size(), 2u);
}

TEST_F(PowerUpManagerTest, FiveCheckpointsSpawnTwoPowerUps) {
    power_ups.load(make_test_race(5, 1).checkpoints);

    EXPECT_EQ(power_ups.get_power_ups().size(), 2u);
}

TEST_F(PowerUpManagerTest, PowerUpSpawnsAtMidpointOfCheckpoints) {
    RaceDTO race = make_test_race(4, 1);
    const CheckpointDTO& p = race.checkpoints[0];
    const CheckpointDTO& q = race.checkpoints[1];

    power_ups.load(race.checkpoints);

    ASSERT_FALSE(power_ups.get_power_ups().empty());
    const PowerUp& first = power_ups.get_power_ups().front();
    EXPECT_FLOAT_EQ(first.get_x(), (static_cast<float>(p.x) + static_cast<float>(q.x)) / 2.0f);
    EXPECT_FLOAT_EQ(first.get_y(), (static_cast<float>(p.y) + static_cast<float>(q.y)) / 2.0f);
}

TEST_F(PowerUpManagerTest, TypesFollowInjectedGenerator) {
    power_ups.load(make_test_race(4, 1).checkpoints);

    ASSERT_EQ(power_ups.get_power_ups().size(), 2u);
    EXPECT_EQ(power_ups.get_power_ups()[0].get_type(), PowerUpType::Shield);
    EXPECT_EQ(power_ups.get_power_ups()[1].get_type(), PowerUpType::Nitro);
}

TEST_F(PowerUpManagerTest, PowerUpFixtureIsSensor) {
    power_ups.load(make_test_race(4, 1).checkpoints);

    ASSERT_FALSE(power_ups.get_power_ups().empty());
    const b2Body* body = power_ups.get_power_ups().front().get_body();
    ASSERT_NE(body, nullptr);
    ASSERT_NE(body->GetFixtureList(), nullptr);
    EXPECT_TRUE(body->GetFixtureList()->IsSensor());
}

TEST_F(PowerUpManagerTest, StaysInactiveBeforeCooldown) {
    power_ups.load(make_test_race(4, 1).checkpoints);
    ASSERT_EQ(power_ups.get_power_ups().size(), 2u);
    ASSERT_TRUE(power_ups.try_collect(0, player));

    step(594);  // 9.9 s

    EXPECT_FALSE(power_ups.get_power_ups()[0].is_active());
}

TEST_F(PowerUpManagerTest, RespawnsAfterCooldownWithNextType) {
    power_ups.load(make_test_race(4, 1).checkpoints);
    ASSERT_EQ(power_ups.get_power_ups().size(), 2u);
    const float x = power_ups.get_power_ups()[0].get_x();
    const float y = power_ups.get_power_ups()[0].get_y();
    ASSERT_TRUE(power_ups.try_collect(0, player));

    step(600);  // 10 s

    const PowerUp& respawned = power_ups.get_power_ups()[0];
    EXPECT_TRUE(respawned.is_active());
    EXPECT_FLOAT_EQ(respawned.get_x(), x);
    EXPECT_FLOAT_EQ(respawned.get_y(), y);
    EXPECT_EQ(respawned.get_type(), PowerUpType::Repair);
}

// Uses the real generator on purpose: it checks the distribution, not a fixed sequence
TEST(PowerUpTypeGeneratorTest, RandomGeneratorYieldsAllThreeTypes) {
    PowerUpTypeGenerator generator = make_random_power_up_type_generator();
    std::set<PowerUpType> seen;

    for (int i = 0; i < 1000; ++i) seen.insert(generator());

    EXPECT_EQ(seen.size(), 3u);
    EXPECT_TRUE(seen.count(PowerUpType::Nitro));
    EXPECT_TRUE(seen.count(PowerUpType::Repair));
    EXPECT_TRUE(seen.count(PowerUpType::Shield));
}
