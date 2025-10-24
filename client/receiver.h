#ifndef RECEIVER_H
#define RECEIVER_H
#include "client_protocol.h"
#include "common/thread.h"

struct GameState {
    // Forward Declaration
};

class Receiver : public Thread {
    ClientProtocol& protocol;
    GameState game_state;
    std::mutex state_mutex;

public:
    explicit Receiver(ClientProtocol& protocol);
    GameState get_game_state() const;
    void run() override;
};

#endif //RECEIVER_H