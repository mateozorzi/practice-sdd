#ifndef RECEIVER_H
#define RECEIVER_H


#include <variant>
#include <vector>

#include "../common/race_dto.h"
#include "../common/world_snapshot_dto.h"
#include "common/car_position_dto.h"
#include "common/queue.h"
#include "common/thread.h"

#include "client_protocol.h"


struct GameState {};


class Receiver: public Thread {

    ClientProtocol& protocol;
    GameState game_state;
    std::mutex state_mutex;
    Queue<GameUpdateDTO> car_positions_queue;
    bool server_connected = true;


public:
    explicit Receiver(ClientProtocol& protocol);

    GameState get_game_state() const;
    Queue<GameUpdateDTO>& get_car_positions_queue();

    bool is_server_connected() const;

    void run() override;
    void stop() override;

    ~Receiver() override;
};


#endif
