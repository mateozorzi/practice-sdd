#ifndef GAME_LOOP_H
#define GAME_LOOP_H


#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/car_position_dto.h"
#include "common/constants.h"
#include "common/movement_dto.h"
#include "common/queue.h"
#include "common/thread.h"

#include "car.h"
#include "client_request.h"
#include "server_protocol.h"


struct GameCommand {

    uint32_t client_id;
    ClientRequest client_request;
    MovementDTO movement_dto;
};


class GameLoop: public Thread {


    std::mutex mtx;
    std::vector<Queue<CarPositionDTO>*> client_queues;
    Queue<GameCommand> command_queue;
    std::unordered_map<uint32_t, Car> cars;
    b2World world;

    void broadcast(const CarPositionDTO& msg);

    void handle_car_movement(const GameCommand& game_command);
    void update_positions();


public:
    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;

    GameLoop();

    void add_command(const GameCommand& cmd);
    void register_client(Queue<CarPositionDTO>* client_queue);
    void unregister_client(Queue<CarPositionDTO>* client_queue);

    void run() override;
    void stop() override;
};


#endif
