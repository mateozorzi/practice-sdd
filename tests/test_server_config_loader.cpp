#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "../server/server_config_loader.h"

#include "server_test_utils.h"

namespace {

const char* GAME_WITHOUT_POWER_UPS = R"(game:
  game_fps: 30
  wait_seconds_upgrade_shop: 1
  wait_seconds_countdown: 2
  timeout_seconds_race: 3
  wait_seconds_leaderboard: 4
  wait_seconds_game_finished: 5
  penalty_per_upgrade: 6
)";

// Power-up values of the Hito 1 Background, nested under `game:`
const char* BACKGROUND_POWER_UPS = R"(  power_ups:
    nitro_multiplier: 2.0
    nitro_duration: 2
    repair_amount: 30
    shield_duration: 4
    respawn_cooldown: 10
)";

// Smallest valid game configuration: the required `power_ups` section is included
std::string minimal_game() { return std::string(GAME_WITHOUT_POWER_UPS) + BACKGROUND_POWER_UPS; }

const char* MINIMAL_PHYSICS = "physics:\n  collidable:\n    density: 2.0\n"
                              "    friction: 0.5\n    restitution: 0.25\n";

const char* MINIMAL_WORLD = R"(world:
  pixels_per_meter: 10
  races_folder: "races/"
  liberty_city: {}
  san_andreas: {}
  vice_city: {}
)";

class ServerConfigLoaderTest: public ::testing::Test {
protected:
    std::filesystem::path config_dir;

    void SetUp() override {
        config_dir = std::filesystem::temp_directory_path() /
                     ("nfs_config_test_" +
                      std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::filesystem::create_directories(config_dir);
    }

    void TearDown() override { std::filesystem::remove_all(config_dir); }

    void write_file(const std::string& name, const std::string& content) {
        std::ofstream(config_dir / name) << content;
    }

    // Message of the error thrown while loading the config written in config_dir
    std::string load_error() {
        try {
            ServerConfigLoader().load_yaml(config_dir.string());
        } catch (const std::runtime_error& error) {
            return error.what();
        }
        ADD_FAILURE() << "expected the configuration loading to fail";
        return "";
    }

    void write_config(const std::string& game, const std::string& physics,
                      const std::string& world) {
        write_file("game_config.yaml", game);
        write_file("physics_config.yaml", physics);
        write_file("world_config.yaml", world);
    }
};

}  // namespace


TEST_F(ServerConfigLoaderTest, LoadsGameSectionFromRepositoryConfig) {
    ServerConfig config = ServerConfigLoader().load_yaml(source_path("server/config"));

    EXPECT_EQ(config.game.game_fps, 60u);
    EXPECT_EQ(config.game.wait_seconds_upgrade_shop, 10u);
    EXPECT_EQ(config.game.wait_seconds_countdown, 3u);
    EXPECT_EQ(config.game.timeout_seconds_race, 600u);
    EXPECT_EQ(config.game.penalty_per_upgrade, 10u);
    EXPECT_FLOAT_EQ(config.game.upgrades.health_upgrade, 50.0f);
    EXPECT_FLOAT_EQ(config.game.upgrades.speed_upgrade, 3.0f);
    EXPECT_FLOAT_EQ(config.game.upgrades.acceleration_upgrade, 5.0f);
    EXPECT_FLOAT_EQ(config.game.upgrades.handling_upgrade, 30.0f);
}

TEST_F(ServerConfigLoaderTest, LoadsServerSectionFromRepositoryConfig) {
    ServerConfig config = ServerConfigLoader().load_yaml(source_path("server/config"));

    EXPECT_EQ(config.server.server_port, 8080);
    EXPECT_EQ(config.server.max_concurrent_games, 3u);
    EXPECT_EQ(config.server.min_players_per_game, 1u);
    EXPECT_EQ(config.server.max_players_per_game, 8u);
    EXPECT_EQ(config.server.command_queue_capacity, 300u);
    EXPECT_EQ(config.server.snapshot_queue_capacity, 100u);
}

TEST_F(ServerConfigLoaderTest, LoadsEveryCarModel) {
    ServerConfig config = ServerConfigLoader().load_yaml(source_path("server/config"));

    EXPECT_EQ(config.physics.compact.model, "Compact");
    EXPECT_EQ(config.physics.sport.model, "Sport");
    EXPECT_EQ(config.physics.convertible.model, "Convertible");
    EXPECT_EQ(config.physics.sedan.model, "Sedan");
    EXPECT_EQ(config.physics.jeep.model, "Jeep");
    EXPECT_EQ(config.physics.rally.model, "Rally");
    EXPECT_EQ(config.physics.truck.model, "Truck");

    EXPECT_EQ(config.physics.compact.health, 30u);
    EXPECT_EQ(config.physics.truck.health, 300u);
    EXPECT_FLOAT_EQ(config.physics.sport.max_speed, 28.0f);
    EXPECT_FLOAT_EQ(config.physics.truck.max_speed, 12.0f);
}

TEST_F(ServerConfigLoaderTest, LoadsWorldPaths) {
    ServerConfig config = ServerConfigLoader().load_yaml(source_path("server/config"));

    EXPECT_EQ(config.world.pixels_per_meter, 30u);
    EXPECT_EQ(config.world.races_folder, "/var/nfs/races/");
    EXPECT_EQ(config.world.liberty_city.npc_file, "/var/nfs/world/npcs/npcs_liberty_city.json");
    EXPECT_EQ(config.world.vice_city.sensors.under_bridge_sensor_file,
              "/var/nfs/world/sensors/under_bridge_sensors_vice_city.json");
}

TEST_F(ServerConfigLoaderTest, OptionalSectionsCanBeOmitted) {
    write_config(minimal_game(), MINIMAL_PHYSICS, MINIMAL_WORLD);

    ServerConfig config = ServerConfigLoader().load_yaml(config_dir.string());

    EXPECT_EQ(config.game.game_fps, 30u);
    EXPECT_EQ(config.game.penalty_per_upgrade, 6u);
    EXPECT_FLOAT_EQ(config.physics.block.density, 2.0f);
    EXPECT_FLOAT_EQ(config.physics.block.restitution, 0.25f);
    EXPECT_EQ(config.world.pixels_per_meter, 10u);
    EXPECT_EQ(config.world.races_folder, "races/");
    EXPECT_TRUE(config.world.liberty_city.npc_file.empty());
}

TEST_F(ServerConfigLoaderTest, MissingDirectoryThrows) {
    EXPECT_THROW(ServerConfigLoader().load_yaml((config_dir / "does_not_exist").string()),
                 std::runtime_error);
}

TEST_F(ServerConfigLoaderTest, MissingGameRootNodeThrows) {
    write_config("not_game:\n  game_fps: 1\n", MINIMAL_PHYSICS, MINIMAL_WORLD);

    EXPECT_THROW(ServerConfigLoader().load_yaml(config_dir.string()), std::runtime_error);
}

TEST_F(ServerConfigLoaderTest, MissingPhysicsRootNodeThrows) {
    write_config(minimal_game(), "something_else: {}\n", MINIMAL_WORLD);

    EXPECT_THROW(ServerConfigLoader().load_yaml(config_dir.string()), std::runtime_error);
}

TEST_F(ServerConfigLoaderTest, MissingWorldRootNodeThrows) {
    write_config(minimal_game(), MINIMAL_PHYSICS, "something_else: {}\n");

    EXPECT_THROW(ServerConfigLoader().load_yaml(config_dir.string()), std::runtime_error);
}

TEST_F(ServerConfigLoaderTest, InvalidValueTypeThrows) {
    std::string game = minimal_game();
    game.replace(game.find("30"), 2, "fast");
    write_config(game, MINIMAL_PHYSICS, MINIMAL_WORLD);

    EXPECT_THROW(ServerConfigLoader().load_yaml(config_dir.string()), std::runtime_error);
}

TEST_F(ServerConfigLoaderTest, LoadsPowerUpsSection) {
    write_config(minimal_game(), MINIMAL_PHYSICS, MINIMAL_WORLD);

    ServerConfig config = ServerConfigLoader().load_yaml(config_dir.string());

    EXPECT_FLOAT_EQ(config.game.power_ups.nitro_multiplier, 2.0f);
    EXPECT_DOUBLE_EQ(config.game.power_ups.nitro_duration, 2.0);
    EXPECT_EQ(config.game.power_ups.repair_amount, 30u);
    EXPECT_DOUBLE_EQ(config.game.power_ups.shield_duration, 4.0);
    EXPECT_DOUBLE_EQ(config.game.power_ups.respawn_cooldown, 10.0);
}

TEST_F(ServerConfigLoaderTest, MissingPowerUpValueThrows) {
    std::string power_ups = BACKGROUND_POWER_UPS;
    power_ups.erase(power_ups.find("    shield_duration"),
                    std::string("    shield_duration: 4\n").size());
    write_config(std::string(GAME_WITHOUT_POWER_UPS) + power_ups, MINIMAL_PHYSICS, MINIMAL_WORLD);

    EXPECT_NE(load_error().find("shield_duration"), std::string::npos);
}

TEST_F(ServerConfigLoaderTest, NegativePowerUpValueThrows) {
    std::string power_ups = BACKGROUND_POWER_UPS;
    power_ups.replace(power_ups.find("repair_amount: 30"), 17, "repair_amount: -30");
    write_config(std::string(GAME_WITHOUT_POWER_UPS) + power_ups, MINIMAL_PHYSICS, MINIMAL_WORLD);

    EXPECT_NE(load_error().find("repair_amount"), std::string::npos);
}

TEST_F(ServerConfigLoaderTest, MissingPowerUpsSectionThrows) {
    write_config(GAME_WITHOUT_POWER_UPS, MINIMAL_PHYSICS, MINIMAL_WORLD);

    EXPECT_NE(load_error().find("power_ups"), std::string::npos);
}
