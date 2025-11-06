#ifndef RECEIVER_H
#define RECEIVER_H
#include <vector>

#include "common/car_position_dto.h"
#include "common/queue.h"
#include "common/thread.h"

#include "client_protocol.h"

struct GameState {
    // Forward Declaration
};

class Receiver: public Thread {
    ClientProtocol& protocol;
    GameState game_state;
    std::mutex state_mutex;
    Queue<std::vector<CarPositionDTO>> car_positions_queue;

public:
    explicit Receiver(ClientProtocol& protocol);
    GameState get_game_state() const;
    Queue<std::vector<CarPositionDTO>>& get_car_positions_queue();
    void run() override;
};

#endif  // RECEIVER_H
