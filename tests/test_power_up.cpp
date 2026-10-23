#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include "../common/power_up_type.h"
#include "../server/city_utils.h"
#include "../server/player.h"
#include "../server/power_up.h"
#include "../server/race_loader.h"
#include "../server/server_constants.h"
#include "../server/streets_grid.h"

#include "server_test_utils.h"
#include "streets_test_utils.h"

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

// Milestone 5: the same Background as milestone 2, with the street grids built in the test
class StreetPowerUpTest: public PowerUpManagerTest {
protected:
    // Checkpoints with ids 1, 2, ... at the given pixels
    static std::vector<CheckpointDTO> checkpoints_at(
            const std::vector<std::pair<uint16_t, uint16_t>>& positions) {
        std::vector<CheckpointDTO> checkpoints;
        uint8_t id = 1;
        for (const auto& [x, y]: positions) checkpoints.emplace_back(id++, 'V', x, y, 15, 34);
        return checkpoints;
    }

    void expect_power_up_at(const size_t index, const float x, const float y) const {
        ASSERT_GT(power_ups.get_power_ups().size(), index);
        const PowerUp& power_up = power_ups.get_power_ups()[index];
        EXPECT_FLOAT_EQ(power_up.get_x(), x) << "power-up " << index;
        EXPECT_FLOAT_EQ(power_up.get_y(), y) << "power-up " << index;
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


TEST_F(StreetPowerUpTest, StraightStreetKeepsMidpoint) {
    const StreetMap street = make_straight_street_map();

    power_ups.load(checkpoints_at({{0, 0}, {200, 0}}), &street);

    expect_power_up_at(0, 100.0f, 0.0f);
}

TEST_F(StreetPowerUpTest, LShapedStreetUsesPathMidpoint) {
    const StreetMap street = make_l_street_map();

    power_ups.load(checkpoints_at({{0, 0}, {200, 200}}), &street);

    expect_power_up_at(0, 200.0f, 0.0f);
}

TEST_F(StreetPowerUpTest, NoPathFallsBackToMidpointAndReportsStretch) {
    const StreetMap streets = make_two_streets_map();

    // The tests' main silences std::cerr, so it is redirected here to check the message
    std::ostringstream errors;
    std::streambuf* silenced = std::cerr.rdbuf(errors.rdbuf());
    power_ups.load(checkpoints_at({{0, 0}, {200, 100}}), &streets);
    std::cerr.rdbuf(silenced);

    expect_power_up_at(0, 100.0f, 50.0f);
    EXPECT_NE(errors.str().find("1 -> 2"), std::string::npos) << errors.str();
}

TEST_F(StreetPowerUpTest, ArbitraryCheckpointsNeverFail) {
    const StreetMap streets = make_two_streets_map();
    const auto checkpoints =
            checkpoints_at({{0, 0}, {200, 0}, {0, 0}, {200, 100}, {1000, 1000}, {0, 100}});

    std::ostringstream errors;
    std::streambuf* silenced = std::cerr.rdbuf(errors.rdbuf());
    EXPECT_NO_THROW(power_ups.load(checkpoints, &streets));
    std::cerr.rdbuf(silenced);

    ASSERT_EQ(power_ups.get_power_ups().size(), 3u);
    expect_power_up_at(0, 100.0f, 0.0f);   // path
    expect_power_up_at(1, 100.0f, 50.0f);  // no path: (P + Q) / 2
    expect_power_up_at(2, 100.0f,
                       100.0f);  // path from (200, 100), the street closest to (1000, 1000)
}

// Fixed copy of the game races and the street grids of server/world (the only milestone 5 test that
// reads them); the config is only used for the paths of the world files
TEST(RealRacesPowerUpTest, AllPowerUpsLandOnStreets) {
    ServerConfig config = make_power_up_test_server_config();
    config.world.races_folder = source_path("tests/data/races/");
    const std::vector<RaceDTO> races = RaceLoader(config).load_races();
    ASSERT_EQ(races.size(), 6u);
    CityUtils city_utils(config);

    for (const RaceDTO& race: races) {
        const CityLoaderConfig& city =
                city_utils.obtain_city_loader_config(city_utils.normalize_city_name(race.city));
        const StreetMap street_map = load_street_map(city, city_utils);

        for (size_t first = 0; first + 1 < race.checkpoints.size(); first += 2) {
            const CheckpointDTO& p = race.checkpoints[first];
            const CheckpointDTO& q = race.checkpoints[first + 1];
            const auto path =
                    find_street_path(street_map, {static_cast<float>(p.x), static_cast<float>(p.y)},
                                     {static_cast<float>(q.x), static_cast<float>(q.y)});
            EXPECT_TRUE(path.has_value())
                    << race.name << ": stretch " << static_cast<int>(p.id) << " -> "
                    << static_cast<int>(q.id) << " has no street path";
        }

        b2World world{b2Vec2(0.0f, 0.0f)};
        PowerUpManager power_ups{world, config,
                                 make_sequence_power_up_type_generator(BACKGROUND_SEQUENCE)};
        power_ups.load(race.checkpoints, &street_map);

        for (const PowerUp& power_up: power_ups.get_power_ups()) {
            const auto cell = street_map.grid.cell_at({power_up.get_x(), power_up.get_y()});
            EXPECT_TRUE(cell.has_value() && street_map.grid.is_street(cell->col, cell->row))
                    << race.name << ": power-up at (" << power_up.get_x() << ", "
                    << power_up.get_y() << ") is not on a street cell";
        }
    }
}
