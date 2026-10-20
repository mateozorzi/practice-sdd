#ifndef SERVER_TEST_UTILS_H
#define SERVER_TEST_UTILS_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "../common/game_update_dto.h"
#include "../common/queue.h"
#include "../common/race_dto.h"
#include "../server/server_config.h"

// Absolute path of a file inside the repository (NFS_SOURCE_DIR is defined by CMake)
std::string source_path(const std::string& relative_path);

// Configuration from server/config with the /var/nfs paths redirected to the repository
ServerConfig make_test_server_config();

// Same as make_test_server_config, but with every wait set to zero so game phases advance fast
ServerConfig make_fast_game_config();

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
class RunningServer {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    RunningServer(const std::string& port, const ServerConfig& config);
    ~RunningServer();
};

#endif
