#ifndef RECEIVER_H
#define RECEIVER_H
#include <vector>

#include "common/car_position_dto.h"
#include "common/queue.h"
#include "common/thread.h"
#include "../common/world_snapshot_dto.h"
#include "../common/race_dto.h"
#include <variant>

#include "client_protocol.h"

struct GameState {
    // Forward Declaration
};

class Receiver: public Thread {
    ClientProtocol& protocol;
    GameState game_state;
    std::mutex state_mutex;
    Queue<std::variant<std::monostate, WorldSnapshotDTO, RaceDTO>> car_positions_queue;

public:
    explicit Receiver(ClientProtocol& protocol);
    GameState get_game_state() const;
    Queue<std::variant<std::monostate, WorldSnapshotDTO, RaceDTO>>& get_car_positions_queue();
    void run() override;
};

#endif  // RECEIVER_H
