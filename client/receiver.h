#ifndef RECEIVER_H
#define RECEIVER_H
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
    Queue<CarPositionDTO> car_position_queue;

public:
    explicit Receiver(ClientProtocol& protocol);
    GameState get_game_state() const;
    Queue<CarPositionDTO>& get_car_position_queue();
    void run() override;
};

#endif  // RECEIVER_H
