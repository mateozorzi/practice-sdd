#ifndef GAME_LOOP_H
#define GAME_LOOP_H


#include <functional>
#include <unordered_map>

#include "../common/thread.h"

#include "car.h"
#include "game_monitor.h"


constexpr float GAME_STEP = 1.0f / 60.0f;
constexpr int VELOCITY_ITERATIONS = 10;
constexpr int POSITION_ITERATIONS = 6;


class GameLoop final: public Thread {


    Queue<GameCommand> command_queue;
    GameMonitor& game_monitor;
    std::unordered_map<uint32_t, Car> cars;
    std::unordered_map<ClientRequest, std::function<void(const GameCommand&)>> command_handlers;
    b2World world;

    void remove_car(uint32_t client_id);

    void process_commands();
    void handle_car_movement(const GameCommand& game_command);
    void handle_disconnection(const GameCommand& game_command);
    void handle_command(const GameCommand& game_command);
    void update_positions();


public:
    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;

    explicit GameLoop(GameMonitor& game_monitor);

    void run() override;
    void stop() override;

    ~GameLoop() override;
};


#endif
