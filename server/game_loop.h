#ifndef GAME_LOOP_H
#define GAME_LOOP_H


#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/constants.h"
#include "common/queue.h"
#include "common/thread.h"


struct GameCommand {

    int client_id;
    uint8_t command;
};


struct GameMessage {

    std::string message;
};


class GameLoop: public Thread {


    Queue<GameCommand> command_queue;
    std::vector<Queue<GameMessage>*> client_queues;
    std::mutex mtx;

    void broadcast(const GameMessage& msg);


public:
    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;

    GameLoop();

    void add_command(const GameCommand& cmd);
    void register_client(Queue<GameMessage>* client_queue);
    void unregister_client(Queue<GameMessage>* client_queue);

    void run() override;
    void stop() override;
};


#endif
