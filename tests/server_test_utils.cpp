#include "server_test_utils.h"

#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "../server/acceptor.h"
#include "../server/lobby.h"
#include "../server/server_config_loader.h"

namespace {

constexpr std::string_view INSTALL_PREFIX = "/var/nfs/";

void redirect_to_repo(std::string& path) {
    if (path.rfind(INSTALL_PREFIX, 0) == 0)
        path = source_path("server/" + path.substr(INSTALL_PREFIX.size()));
}

void redirect_city(CityLoaderConfig& city) {
    redirect_to_repo(city.collidables.on_ground_collidable_file);
    redirect_to_repo(city.collidables.on_bridge_collidable_file);
    redirect_to_repo(city.sensors.on_bridge_sensor_file);
    redirect_to_repo(city.sensors.under_bridge_sensor_file);
    redirect_to_repo(city.objects.bridge_file);
    redirect_to_repo(city.objects.decoration_file);
    redirect_to_repo(city.npc_file);
}

}  // namespace


std::string source_path(const std::string& relative_path) {
    return std::string(NFS_SOURCE_DIR) + "/" + relative_path;
}

ServerConfig make_test_server_config() {
    ServerConfig config = ServerConfigLoader().load_yaml(source_path("server/config"));

    redirect_to_repo(config.world.races_folder);
    redirect_city(config.world.liberty_city);
    redirect_city(config.world.san_andreas);
    redirect_city(config.world.vice_city);

    return config;
}

ServerConfig make_fast_game_config() {
    ServerConfig config = make_test_server_config();

    config.game.game_fps = 200;
    config.game.wait_seconds_upgrade_shop = 0;
    config.game.wait_seconds_countdown = 0;
    config.game.wait_seconds_leaderboard = 0;
    config.game.wait_seconds_game_finished = 0;
    config.server.snapshot_queue_capacity = 100000;

    return config;
}

PowerUpConfig make_test_power_up_config() {
    PowerUpConfig power_ups;

    power_ups.nitro_multiplier = 2.0f;
    power_ups.nitro_duration = 2.0;
    power_ups.repair_amount = 30;
    power_ups.shield_duration = 4.0;
    power_ups.respawn_cooldown = 10.0;

    return power_ups;
}

ServerConfig make_power_up_test_server_config() {
    ServerConfig config = make_test_server_config();

    config.physics.compact.max_speed = 10.0f;
    config.physics.compact.acceleration = 5.0f;
    config.physics.compact.health = 100;
    config.game.upgrades.health_upgrade = 50.0f;
    config.game.upgrades.speed_upgrade = 3.0f;
    config.game.power_ups = make_test_power_up_config();

    return config;
}

ServerConfig make_fast_power_up_game_config() {
    ServerConfig config = make_fast_game_config();

    config.game.timeout_seconds_race = 60;
    config.game.power_ups.nitro_multiplier = 2.0f;
    config.game.power_ups.nitro_duration = 1.0;
    config.game.power_ups.repair_amount = 30;
    config.game.power_ups.shield_duration = 1.0;
    config.game.power_ups.respawn_cooldown = 2.0;

    return config;
}

std::function<PowerUpType()> make_sequence_power_up_type_generator(
        std::vector<PowerUpType> sequence) {
    return [sequence = std::move(sequence), next = size_t{0}]() mutable {
        PowerUpType type = sequence[next];
        next = (next + 1) % sequence.size();
        return type;
    };
}

RaceDTO make_test_race(uint8_t checkpoints, uint8_t laps, const std::string& name) {
    std::vector<CheckpointDTO> race_checkpoints;
    for (uint8_t id = 1; id <= checkpoints; ++id)
        race_checkpoints.emplace_back(id, 'V', static_cast<uint16_t>(1700 + 70 * id), 2391, 15, 66);

    return RaceDTO(name, "Liberty City", race_checkpoints, laps);
}

std::optional<GameUpdateDTO> wait_for_update(
        Queue<GameUpdateDTO>& queue, const std::function<bool(const GameUpdateDTO&)>& predicate,
        std::chrono::milliseconds timeout) {

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    GameUpdateDTO update;

    while (std::chrono::steady_clock::now() < deadline) {
        try {
            if (queue.try_pop(update)) {
                if (predicate(update))
                    return update;
                continue;
            }
        } catch (const ClosedQueue&) {
            return std::nullopt;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return std::nullopt;
}

std::optional<GameUpdateDTO> wait_for_action(Queue<GameUpdateDTO>& queue, GameLoopAction action,
                                             std::chrono::milliseconds timeout) {
    return wait_for_update(
            queue,
            [action](const GameUpdateDTO& update) { return update.gameloop_action == action; },
            timeout);
}


struct RunningServer::Impl {
    Lobby lobby;
    Acceptor acceptor;

    Impl(const std::string& port, const ServerConfig& config,
         std::function<std::function<PowerUpType()>()> power_up_generators):
            lobby(config, power_up_generators ? std::move(power_up_generators) :
                                                make_random_power_up_type_generator),
            acceptor(port, lobby, config) {}
};

RunningServer::RunningServer(const std::string& port, const ServerConfig& config,
                             std::function<std::function<PowerUpType()>()> power_up_generators):
        impl(std::make_unique<Impl>(port, config, std::move(power_up_generators))) {
    impl->acceptor.start();
}

RunningServer::~RunningServer() {
    impl->acceptor.stop();
    impl->acceptor.join();
}
