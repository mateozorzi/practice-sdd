#ifndef SERVER_TEST_UTILS_H
#define SERVER_TEST_UTILS_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../common/game_update_dto.h"
#include "../common/power_up_type.h"
#include "../common/queue.h"
#include "../common/race_dto.h"
#include "../server/server_config.h"

// Absolute path of a file inside the repository (NFS_SOURCE_DIR is defined by CMake)
std::string source_path(const std::string& relative_path);

// Configuration from server/config with the /var/nfs paths redirected to the repository
ServerConfig make_test_server_config();

// Same as make_test_server_config, but with every wait set to zero so game phases advance fast
ServerConfig make_fast_game_config();

// Power-up values of the milestone Background, different on purpose from server/config defaults
PowerUpConfig make_test_power_up_config();

// make_test_server_config with the Background test car (Compact: max speed 10, acceleration 5,
// health 100), upgrades (health 50, speed 3) and make_test_power_up_config()
ServerConfig make_power_up_test_server_config();

// make_fast_game_config with the power-up values of the game loop milestone Background (short
// durations so the loop ends fast) and a race timeout that does not come from server/config
ServerConfig make_fast_power_up_game_config();

// Fake power-up type generator (same type as PowerUpTypeGenerator, spelled out so this header does
// not include server/power_up.h): returns `sequence` in order, starting over when it runs out
std::function<PowerUpType()> make_sequence_power_up_type_generator(
        std::vector<PowerUpType> sequence);

// Race in Liberty City with `checkpoints` checkpoints placed one after the other
RaceDTO make_test_race(uint8_t checkpoints, uint8_t laps, const std::string& name = "test_race");

// Pops updates until one matches `predicate`. Returns nullopt on timeout or closed queue.
std::optional<GameUpdateDTO> wait_for_update(
        Queue<GameUpdateDTO>& queue, const std::function<bool(const GameUpdateDTO&)>& predicate,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

// Pops updates until one with `action` arrives. Returns nullopt on timeout or closed queue.
std::optional<GameUpdateDTO> wait_for_action(
        Queue<GameUpdateDTO>& queue, GameLoopAction action,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

// Acceptor and lobby listening on localhost:`port` until destroyed. It hides the server headers
// so tests can also include the client ones (both declare a global `Car`).
// `power_up_generators` creates the power-up type generator of each game (empty: the real one).
class RunningServer {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    RunningServer(const std::string& port, const ServerConfig& config,
                  std::function<std::function<PowerUpType()>()> power_up_generators = {});
    ~RunningServer();
};

#endif
