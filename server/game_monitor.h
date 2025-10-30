#ifndef GAME_MONITOR_H
#define GAME_MONITOR_H


#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../common/car_position_dto.h"
#include "../common/queue.h"

#include "game_command.h"


class GameMonitor {


    std::mutex mtx;
    std::unordered_map<uint32_t, std::unique_ptr<Queue<CarPositionDTO>>> client_queues;
    std::optional<std::reference_wrapper<Queue<GameCommand>>> command_queue_reference;


public:
    GameMonitor(const GameMonitor&) = delete;
    GameMonitor& operator=(const GameMonitor&) = delete;

    GameMonitor() = default;

    Queue<CarPositionDTO>& register_client(uint32_t client_id);
    void unregister_client(uint32_t client_id);
    void broadcast(const CarPositionDTO& car_position_dto);
    void remove_client(uint32_t client_id);

    void register_command_queue(Queue<GameCommand>& command_queue);
    void add_command(const GameCommand& command) const;

    ~GameMonitor();
};


#endif
