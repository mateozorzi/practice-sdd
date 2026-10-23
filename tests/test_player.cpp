#include <cmath>
#include <string>
#include <vector>

#include <box2d/box2d.h>
#include <gtest/gtest.h>

#include "../common/constants.h"
#include "../common/power_up_type.h"
#include "../server/player.h"
#include "../server/server_constants.h"

#include "server_test_utils.h"

namespace {

constexpr uint32_t PLAYER_ID = 7;

class PlayerTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    b2World world{b2Vec2(0.0f, 0.0f)};
    Player player{PLAYER_ID, "mateo"};

    void create_car(const std::string& model = "Compact") {
        player.create_car(model, config, world);
    }

    // Triggers a checkpoint and lets the player process it, as the game loop does every tick
    void cross_checkpoint(const RaceDTO& race, uint32_t elapsed_seconds = 0) {
        player.trigger_checkpoint_collision();
        player.update_car_position(race, elapsed_seconds, config.game);
    }
};

CarPositionDTO start_position(const ServerConfig& config, const CheckpointDTO& first,
                              const CheckpointDTO& second) {
    b2World world(b2Vec2(0.0f, 0.0f));
    Car car(1, world, "Compact", config);
    car.set_position_for_race({first, second});
    return car.get_position_dto();
}

}  // namespace


TEST_F(PlayerTest, PlayerWithoutCarHasDefaults) {
    RaceDTO race = make_test_race(2, 1);

    EXPECT_EQ(player.get_id(), PLAYER_ID);
    EXPECT_EQ(player.get_username(), "mateo");
    EXPECT_EQ(player.get_car().model, "");
    EXPECT_EQ(player.get_car_body(), nullptr);
    EXPECT_EQ(player.get_car_layer(), CarLayer::ON_GROUND);
    EXPECT_EQ(player.get_car_position(race).client_id, PLAYER_ID);
    EXPECT_EQ(player.get_health(), 0);
    EXPECT_FALSE(player.is_in_game());
    EXPECT_FALSE(player.is_in_race());
    EXPECT_EQ(player.get_upgrades_remaining(), MAX_UPGRADES_PER_RACE);

    // Without a car these are no-ops
    player.handle_car_movement(MovementDTO(Direction::UP, true));
    player.set_car_collision(true);
    player.set_car_layer(CarLayer::ON_BRIDGE);
    player.set_upgrade(BuyUpgradeDTO{ButtonType::Speed}, config.game.upgrades);
    EXPECT_EQ(player.get_upgrades_remaining(), MAX_UPGRADES_PER_RACE);
}

TEST_F(PlayerTest, CreateCarUsesTheModelConfiguration) {
    create_car("Truck");

    EXPECT_NE(player.get_car_body(), nullptr);
    EXPECT_EQ(player.get_car().model, "Truck");
    EXPECT_EQ(player.get_health(), config.physics.truck.health);
    EXPECT_EQ(player.get_info().max_health, config.physics.truck.health);
    EXPECT_FLOAT_EQ(player.get_car_body()->GetMass(), config.physics.truck.mass);
}

TEST_F(PlayerTest, EveryModelCanBeCreated) {
    for (const CarConfig* car_config:
         {&config.physics.compact, &config.physics.sport, &config.physics.convertible,
          &config.physics.sedan, &config.physics.jeep, &config.physics.rally,
          &config.physics.truck}) {
        Car car(1, world, car_config->model, config);
        EXPECT_EQ(car.get_configuration().model, car_config->model);
        EXPECT_EQ(car.get_max_health(), car_config->health);
    }
}

TEST_F(PlayerTest, UnknownModelFallsBackToCompact) {
    create_car("Batmobile");

    EXPECT_EQ(player.get_car().model, config.physics.compact.model);
}

TEST_F(PlayerTest, InGameFlag) {
    player.set_in_game();

    EXPECT_TRUE(player.is_in_game());
}

TEST_F(PlayerTest, HealthUpgradeRaisesCurrentAndMaxHealth) {
    create_car();
    const uint16_t base = player.get_health();

    player.set_upgrade(BuyUpgradeDTO{ButtonType::Health}, config.game.upgrades);

    const uint16_t expected = base + static_cast<uint16_t>(config.game.upgrades.health_upgrade);
    EXPECT_EQ(player.get_health(), expected);
    EXPECT_EQ(player.get_info().max_health, expected);
    EXPECT_EQ(player.get_upgrades_remaining(), MAX_UPGRADES_PER_RACE - 1);
}

TEST_F(PlayerTest, UpgradesAreLimitedPerRace) {
    create_car();

    for (int i = 0; i < MAX_UPGRADES_PER_RACE + 2; ++i)
        player.set_upgrade(BuyUpgradeDTO{ButtonType::Health}, config.game.upgrades);

    EXPECT_EQ(player.get_upgrades_remaining(), 0);
    EXPECT_EQ(player.get_health(),
              config.physics.compact.health +
                      MAX_UPGRADES_PER_RACE *
                              static_cast<uint16_t>(config.game.upgrades.health_upgrade));
}

TEST_F(PlayerTest, UpgradesAddPenaltyToRecordedTime) {
    create_car();
    player.set_upgrade(BuyUpgradeDTO{ButtonType::Speed}, config.game.upgrades);
    player.set_upgrade(BuyUpgradeDTO{ButtonType::Acceleration}, config.game.upgrades);

    player.record_time(100, config.game);

    EXPECT_EQ(player.get_recorded_time(), 100 + 2 * config.game.penalty_per_upgrade);
    EXPECT_TRUE(player.have_time_recorded());

    // The penalty is charged only once
    player.record_time(50, config.game);
    EXPECT_EQ(player.get_recorded_time(), 50u);
}

TEST_F(PlayerTest, NoTimeRecordedByDefault) {
    EXPECT_FALSE(player.have_time_recorded());
    EXPECT_EQ(player.get_race_stats().total_time, 0u);
    EXPECT_EQ(player.get_race_stats().client_id, PLAYER_ID);
    EXPECT_EQ(player.get_race_stats().username, "mateo");
}

TEST_F(PlayerTest, CollisionDamageReducesHealthUntilZero) {
    create_car();
    const uint16_t base = player.get_health();

    player.apply_collision_damage(10);
    EXPECT_EQ(player.get_health(), base - 10);
    EXPECT_FALSE(player.is_dead());

    player.apply_collision_damage(base);
    EXPECT_EQ(player.get_health(), 0);
    EXPECT_TRUE(player.is_dead());
}

TEST_F(PlayerTest, InfiniteHealthIgnoresDamage) {
    create_car();
    const uint16_t base = player.get_health();

    player.activate_infinite_health();
    player.apply_collision_damage(base);

    EXPECT_EQ(player.get_health(), base);
}

TEST_F(PlayerTest, StartRaceResetsTheRaceState) {
    create_car();
    player.apply_collision_damage(5);
    player.activate_infinite_health();

    player.start_race();

    EXPECT_TRUE(player.is_in_race());
    EXPECT_FALSE(player.has_finished_race());
    EXPECT_TRUE(player.is_next_checkpoint(1));
    EXPECT_EQ(player.get_health(), config.physics.compact.health);

    // Infinite health was reset by start_race
    player.apply_collision_damage(5);
    EXPECT_EQ(player.get_health(), config.physics.compact.health - 5);
}

TEST_F(PlayerTest, CheckpointsAreIgnoredOutsideRace) {
    create_car();
    RaceDTO race = make_test_race(3, 1);
    player.set_position_for_race(race.checkpoints);

    cross_checkpoint(race);

    EXPECT_TRUE(player.is_next_checkpoint(1));
}

TEST_F(PlayerTest, CrossingCheckpointsAdvancesLapsAndFinishesRace) {
    create_car();
    RaceDTO race = make_test_race(3, 2);
    player.set_position_for_race(race.checkpoints);
    player.start_race();

    EXPECT_EQ(player.get_car_position(race).next.id, 1);
    EXPECT_EQ(player.get_car_position(race).current_lap, 1);
    EXPECT_EQ(player.get_car_position(race).race_lap, 2);

    cross_checkpoint(race);
    EXPECT_TRUE(player.is_next_checkpoint(2));
    cross_checkpoint(race);
    EXPECT_TRUE(player.is_next_checkpoint(3));
    EXPECT_FALSE(player.get_car_position(race).next.is_finish_line);

    // Third checkpoint closes the first lap
    cross_checkpoint(race);
    EXPECT_TRUE(player.is_next_checkpoint(1));
    EXPECT_EQ(player.get_car_position(race).current_lap, 2);

    cross_checkpoint(race);
    cross_checkpoint(race);
    EXPECT_TRUE(player.is_next_checkpoint(3));
    EXPECT_TRUE(player.get_car_position(race).next.is_finish_line);
    EXPECT_FALSE(player.has_finished_race());

    cross_checkpoint(race, 42);
    EXPECT_TRUE(player.has_finished_race());
    EXPECT_FALSE(player.is_in_race());
    EXPECT_EQ(player.get_recorded_time(), 42u);
    EXPECT_TRUE(player.get_info().finish_race);
}

TEST_F(PlayerTest, FinishedPlayerTakesNoDamage) {
    create_car();
    player.start_race();
    player.finish_race();

    player.apply_collision_damage(5);

    EXPECT_EQ(player.get_health(), config.physics.compact.health);
}

TEST_F(PlayerTest, PlayerWithoutHealthIsMarkedDeadOnUpdate) {
    create_car();
    RaceDTO race = make_test_race(2, 1);
    player.set_position_for_race(race.checkpoints);
    player.start_race();

    player.apply_collision_damage(player.get_health());
    player.update_car_position(race, 1, config.game);

    EXPECT_FALSE(player.is_in_race());
    EXPECT_TRUE(player.get_info().dead);
}

TEST_F(PlayerTest, MovementIsOnlyAppliedDuringRace) {
    create_car();
    RaceDTO race = make_test_race(2, 1);

    player.handle_car_movement(MovementDTO(Direction::UP, true));
    player.update_car_position(race, 0, config.game);
    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    EXPECT_FLOAT_EQ(player.get_car_body()->GetLinearVelocity().Length(), 0.0f);

    player.start_race();
    player.handle_car_movement(MovementDTO(Direction::UP, true));
    player.update_car_position(race, 0, config.game);
    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    EXPECT_GT(player.get_car_body()->GetLinearVelocity().Length(), 0.0f);
}

TEST_F(PlayerTest, FinishRaceStopsTheCar) {
    create_car();
    player.start_race();
    player.get_car_body()->SetLinearVelocity(b2Vec2(3.0f, 4.0f));
    player.get_car_body()->SetAngularVelocity(1.0f);

    player.finish_race();

    EXPECT_FLOAT_EQ(player.get_car_body()->GetLinearVelocity().Length(), 0.0f);
    EXPECT_FLOAT_EQ(player.get_car_body()->GetAngularVelocity(), 0.0f);
}

TEST_F(PlayerTest, CarLayerChangesTheCollisionFilter) {
    create_car();
    b2Fixture* fixture = player.get_car_body()->GetFixtureList();

    player.set_car_layer(CarLayer::ON_BRIDGE);
    EXPECT_EQ(player.get_car_layer(), CarLayer::ON_BRIDGE);
    EXPECT_EQ(fixture->GetFilterData().categoryBits, CATEGORY_CAR_BRIDGE);
    EXPECT_EQ(fixture->GetFilterData().maskBits, MASK_CAR_BRIDGE);

    player.set_car_layer(CarLayer::UNDER_BRIDGE);
    EXPECT_EQ(fixture->GetFilterData().categoryBits, CATEGORY_CAR_UNDER);
    EXPECT_EQ(fixture->GetFilterData().maskBits, MASK_CAR_UNDER);

    player.set_car_layer(CarLayer::ON_GROUND);
    EXPECT_EQ(fixture->GetFilterData().categoryBits, CATEGORY_CAR_GROUND);
    EXPECT_EQ(fixture->GetFilterData().maskBits, MASK_CAR_GROUND);
}


// Car

TEST(CarStartPositionTest, HorizontalCheckpointFacesTheNextOne) {
    ServerConfig config = make_test_server_config();
    const uint32_t ppm = config.world.pixels_per_meter;

    CarPositionDTO going_down = start_position(config, CheckpointDTO(1, HORIZONTAL, 300, 600),
                                               CheckpointDTO(2, 'H', 300, 900));
    EXPECT_EQ(going_down.angle, 180u);
    EXPECT_NEAR(going_down.x_position, 300, 1);
    EXPECT_NEAR(going_down.y_position, 600 - ppm, 1);

    CarPositionDTO going_up = start_position(config, CheckpointDTO(1, HORIZONTAL, 300, 600),
                                             CheckpointDTO(2, 'H', 300, 100));
    EXPECT_EQ(going_up.angle, 0u);
    EXPECT_NEAR(going_up.y_position, 600 + ppm, 1);
}

TEST(CarStartPositionTest, VerticalCheckpointFacesTheNextOne) {
    ServerConfig config = make_test_server_config();
    const uint32_t ppm = config.world.pixels_per_meter;

    CarPositionDTO going_left = start_position(config, CheckpointDTO(1, VERTICAL, 600, 300),
                                               CheckpointDTO(2, 'V', 100, 300));
    EXPECT_EQ(going_left.angle, 270u);
    EXPECT_NEAR(going_left.x_position, 600 + ppm, 1);

    CarPositionDTO going_right = start_position(config, CheckpointDTO(1, VERTICAL, 600, 300),
                                                CheckpointDTO(2, 'V', 900, 300));
    EXPECT_EQ(going_right.angle, 90u);
    EXPECT_NEAR(going_right.x_position, 600 - ppm, 1);
}

TEST(CarStartPositionTest, IntersectionCheckpointUsesTheRelativePosition) {
    ServerConfig config = make_test_server_config();
    const CheckpointDTO start(1, INTERSECTION, 600, 600);

    EXPECT_EQ(start_position(config, start, CheckpointDTO(2, 'H', 600, 900)).angle, 180u);
    EXPECT_EQ(start_position(config, start, CheckpointDTO(2, 'H', 600, 300)).angle, 0u);
    EXPECT_EQ(start_position(config, start, CheckpointDTO(2, 'V', 300, 600)).angle, 270u);
    EXPECT_EQ(start_position(config, start, CheckpointDTO(2, 'V', 900, 600)).angle, 90u);
}

class CarTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    b2World world{b2Vec2(0.0f, 0.0f)};
    Car car{1, world, "Compact", config};
};

TEST_F(CarTest, SpeedIsLimitedToMaxSpeed) {
    car.get_body()->SetLinearVelocity(b2Vec2(0.0f, -100.0f));

    car.update_position();

    EXPECT_NEAR(car.get_body()->GetLinearVelocity().Length(), config.physics.compact.max_speed,
                1e-3);
}

TEST_F(CarTest, SpeedUpgradeRaisesTheLimit) {
    car.set_upgrade(BuyUpgradeDTO{ButtonType::Speed}, config.game.upgrades);
    car.get_body()->SetLinearVelocity(b2Vec2(0.0f, -100.0f));

    car.update_position();

    EXPECT_NEAR(car.get_body()->GetLinearVelocity().Length(),
                config.physics.compact.max_speed + config.game.upgrades.speed_upgrade, 1e-3);
}

TEST_F(CarTest, DriftingDampsLateralVelocity) {
    car.get_body()->SetLinearVelocity(b2Vec2(1.0f, 0.0f));

    car.update_position();

    EXPECT_NEAR(car.get_body()->GetLinearVelocity().x, 1.0f - config.physics.compact.drift_factor,
                1e-4);
}

TEST_F(CarTest, HandlingUpgradeReducesDrift) {
    car.set_upgrade(BuyUpgradeDTO{ButtonType::Handling}, config.game.upgrades);
    car.get_body()->SetLinearVelocity(b2Vec2(1.0f, 0.0f));

    car.update_position();

    EXPECT_NEAR(car.get_body()->GetLinearVelocity().x,
                1.0f - (config.physics.compact.drift_factor - 0.05f), 1e-4);
}

TEST_F(CarTest, AccelerationUpgradeAcceleratesFaster) {
    b2World other_world(b2Vec2(0.0f, 0.0f));
    Car upgraded(2, other_world, "Compact", config);
    upgraded.set_upgrade(BuyUpgradeDTO{ButtonType::Acceleration}, config.game.upgrades);

    for (Car* each: {&car, &upgraded}) each->handle_movement(MovementDTO(Direction::UP, true));
    car.update_position();
    upgraded.update_position();
    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    other_world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

    EXPECT_GT(upgraded.get_body()->GetLinearVelocity().Length(),
              car.get_body()->GetLinearVelocity().Length());
}

TEST_F(CarTest, BrakingPushesTheCarBackwards) {
    car.handle_movement(MovementDTO(Direction::DOWN, true));

    car.update_position();
    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

    // Forward is -y, so braking from rest moves the car towards +y
    EXPECT_GT(car.get_body()->GetLinearVelocity().y, 0.0f);
}

TEST_F(CarTest, TurningRequiresMinimumSpeed) {
    car.handle_movement(MovementDTO(Direction::LEFT, true));
    car.update_position();
    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    EXPECT_FLOAT_EQ(car.get_body()->GetAngularVelocity(), 0.0f);

    car.get_body()->SetLinearVelocity(b2Vec2(0.0f, -config.physics.compact.max_speed));
    car.update_position();
    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    EXPECT_LT(car.get_body()->GetAngularVelocity(), 0.0f);

    car.handle_movement(MovementDTO(Direction::LEFT, false));
    car.handle_movement(MovementDTO(Direction::RIGHT, true));
    car.get_body()->SetAngularVelocity(0.0f);
    car.update_position();
    world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    EXPECT_GT(car.get_body()->GetAngularVelocity(), 0.0f);
}

TEST_F(CarTest, PositionAngleIsNormalizedToDegrees) {
    car.get_body()->SetTransform(b2Vec2(1.0f, 2.0f), -b2_pi / 2.0f);

    CarPositionDTO position = car.get_position_dto();

    EXPECT_EQ(position.client_id, 1u);
    EXPECT_EQ(position.angle, 270u);
    EXPECT_NEAR(position.x_position, config.world.pixels_per_meter, 1);
    EXPECT_NEAR(position.y_position, 2 * config.world.pixels_per_meter, 1);
    EXPECT_EQ(position.layer, CarLayer::ON_GROUND);

    car.get_body()->SetTransform(b2Vec2(1.0f, 2.0f), 2.0f * b2_pi - 0.001f);
    EXPECT_EQ(car.get_position_dto().angle, 0u);
}


// Power-up effects (Hito 1). Every value comes from the Background, never from server/config.

class PowerUpEffectsTest: public ::testing::Test {
protected:
    // Declared before the player: the car keeps a reference to it
    ServerConfig config = make_power_up_test_server_config();
    PowerUpConfig power_ups = config.game.power_ups;
    b2World world{b2Vec2(0.0f, 0.0f)};
    Player player{PLAYER_ID, "mateo"};

    void SetUp() override { player.create_car("Compact", config, world); }

    void take(PowerUpType type) { player.apply_power_up(type, power_ups); }

    bool has_no_effects() const {
        return !player.has_effect(PowerUpType::Nitro) && !player.has_effect(PowerUpType::Shield);
    }
};


TEST_F(PowerUpEffectsTest, NitroDoublesSpeedAndAcceleration) {
    take(PowerUpType::Nitro);

    EXPECT_TRUE(player.has_effect(PowerUpType::Nitro));
    EXPECT_FLOAT_EQ(player.get_max_speed(), 20.0f);
    EXPECT_FLOAT_EQ(player.get_acceleration(), 10.0f);
}

TEST_F(PowerUpEffectsTest, NitroAppliesOverSpeedUpgrade) {
    player.set_upgrade(BuyUpgradeDTO{ButtonType::Speed}, config.game.upgrades);
    ASSERT_FLOAT_EQ(player.get_max_speed(), 13.0f);

    take(PowerUpType::Nitro);

    EXPECT_FLOAT_EQ(player.get_max_speed(), 26.0f);
}

TEST_F(PowerUpEffectsTest, NitroExpiresAfterDurationRestoringValues) {
    take(PowerUpType::Nitro);

    player.advance_effects(1.9);
    EXPECT_TRUE(player.has_effect(PowerUpType::Nitro));
    EXPECT_FLOAT_EQ(player.get_max_speed(), 20.0f);

    player.advance_effects(0.1);
    EXPECT_FALSE(player.has_effect(PowerUpType::Nitro));
    EXPECT_EQ(player.get_max_speed(), 10.0f);
    EXPECT_EQ(player.get_acceleration(), 5.0f);
}

TEST_F(PowerUpEffectsTest, NitroRenewsDurationWithoutStacking) {
    take(PowerUpType::Nitro);
    player.advance_effects(1.0);

    take(PowerUpType::Nitro);

    EXPECT_NEAR(player.get_effect_remaining(PowerUpType::Nitro), 2.0, 1e-9);
    EXPECT_FLOAT_EQ(player.get_max_speed(), 20.0f);
}

TEST_F(PowerUpEffectsTest, NitroAndShieldExpireIndependently) {
    take(PowerUpType::Nitro);
    take(PowerUpType::Shield);

    player.advance_effects(2.0);
    EXPECT_FALSE(player.has_effect(PowerUpType::Nitro));
    EXPECT_TRUE(player.has_effect(PowerUpType::Shield));

    player.advance_effects(2.0);
    EXPECT_TRUE(has_no_effects());
}

TEST_F(PowerUpEffectsTest, RepairAddsHealth) {
    player.apply_collision_damage(50);

    take(PowerUpType::Repair);

    EXPECT_EQ(player.get_health(), 80);
    EXPECT_FALSE(player.has_effect(PowerUpType::Repair));
}

TEST_F(PowerUpEffectsTest, RepairCapsAtMaxHealth) {
    player.apply_collision_damage(10);

    take(PowerUpType::Repair);

    EXPECT_EQ(player.get_health(), 100);
}

TEST_F(PowerUpEffectsTest, RepairCapsAtUpgradedMaxHealth) {
    player.set_upgrade(BuyUpgradeDTO{ButtonType::Health}, config.game.upgrades);
    player.apply_collision_damage(20);
    ASSERT_EQ(player.get_health(), 130);

    take(PowerUpType::Repair);

    EXPECT_EQ(player.get_health(), 150);
}

TEST_F(PowerUpEffectsTest, ShieldIgnoresCollisionDamage) {
    take(PowerUpType::Shield);

    player.apply_collision_damage(20);

    EXPECT_EQ(player.get_health(), 100);
}

TEST_F(PowerUpEffectsTest, ShieldExpiredTakesDamage) {
    take(PowerUpType::Shield);
    ASSERT_TRUE(player.has_effect(PowerUpType::Shield));

    player.advance_effects(4.0);
    player.apply_collision_damage(20);

    EXPECT_EQ(player.get_health(), 80);
}

TEST_F(PowerUpEffectsTest, InfiniteHealthSurvivesShieldExpiry) {
    player.activate_infinite_health();
    take(PowerUpType::Shield);
    ASSERT_TRUE(player.has_effect(PowerUpType::Shield));

    player.advance_effects(4.0);
    player.apply_collision_damage(20);

    EXPECT_EQ(player.get_health(), 100);
}

TEST_F(PowerUpEffectsTest, InfiniteHealthSurvivesRepair) {
    player.activate_infinite_health();
    take(PowerUpType::Repair);

    player.apply_collision_damage(20);

    EXPECT_EQ(player.get_health(), 100);
}

TEST_F(PowerUpEffectsTest, DeadCarIgnoresPowerUps) {
    player.apply_collision_damage(100);
    ASSERT_TRUE(player.is_dead());

    take(PowerUpType::Nitro);
    take(PowerUpType::Repair);
    take(PowerUpType::Shield);

    EXPECT_TRUE(has_no_effects());
    EXPECT_EQ(player.get_health(), 0);
    EXPECT_EQ(player.get_max_speed(), 10.0f);
}

TEST_F(PowerUpEffectsTest, ShieldPreventsLethalDamage) {
    take(PowerUpType::Nitro);
    take(PowerUpType::Shield);

    player.apply_collision_damage(100);

    EXPECT_FALSE(player.is_dead());
    EXPECT_EQ(player.get_health(), 100);
    EXPECT_TRUE(player.has_effect(PowerUpType::Nitro));
    EXPECT_TRUE(player.has_effect(PowerUpType::Shield));
}

TEST_F(PowerUpEffectsTest, DyingClearsEffects) {
    take(PowerUpType::Shield);
    player.advance_effects(3.0);
    take(PowerUpType::Nitro);
    player.advance_effects(1.0);
    ASSERT_FALSE(player.has_effect(PowerUpType::Shield));
    ASSERT_TRUE(player.has_effect(PowerUpType::Nitro));

    player.apply_collision_damage(100);

    EXPECT_TRUE(player.is_dead());
    EXPECT_TRUE(has_no_effects());
    EXPECT_EQ(player.get_max_speed(), 10.0f);
    EXPECT_EQ(player.get_acceleration(), 5.0f);
}

TEST_F(PowerUpEffectsTest, StartRaceClearsEffects) {
    take(PowerUpType::Nitro);
    take(PowerUpType::Shield);
    ASSERT_TRUE(player.has_effect(PowerUpType::Nitro));
    ASSERT_TRUE(player.has_effect(PowerUpType::Shield));

    player.start_race();

    EXPECT_TRUE(has_no_effects());
    EXPECT_EQ(player.get_max_speed(), 10.0f);
    EXPECT_EQ(player.get_acceleration(), 5.0f);
}

TEST_F(PowerUpEffectsTest, FinishRaceClearsEffects) {
    player.start_race();
    take(PowerUpType::Nitro);
    take(PowerUpType::Shield);
    ASSERT_FALSE(has_no_effects());

    player.finish_race();

    EXPECT_TRUE(has_no_effects());
    EXPECT_EQ(player.get_max_speed(), 10.0f);
    EXPECT_EQ(player.get_acceleration(), 5.0f);
}

// The game loop calls it on every player when the race ends, also for the ones that did not
// finish (race timeout)
TEST_F(PowerUpEffectsTest, ClearEffectsRemovesEffectsAndRestoresValues) {
    take(PowerUpType::Nitro);
    take(PowerUpType::Shield);
    ASSERT_FALSE(has_no_effects());

    player.clear_effects();

    EXPECT_TRUE(has_no_effects());
    EXPECT_EQ(player.get_max_speed(), 10.0f);
    EXPECT_EQ(player.get_acceleration(), 5.0f);
}
