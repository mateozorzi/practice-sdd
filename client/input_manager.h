#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#include <SDL2/SDL_events.h>

#include "sender.h"


class InputManager {
private:
    Sender sender;

    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;

    bool handle_keydown(const SDL_Event& event);
    bool handle_keyup(const SDL_Event& event);
    bool try_move(bool& movement, MovementDTO move);
    bool try_stop(bool& movement, MovementDTO move);

public:
    explicit InputManager(ClientProtocol& protocol);
    bool handle_movement();
    bool is_moving() const;
    ~InputManager();
};


#endif
