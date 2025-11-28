#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#include <SDL2/SDL_events.h>

#include "sdl_manager.h"
#include "sender.h"
#include "upgrade_shop.h"


class InputManager {
private:
    SDLManager& sdl;
    Sender sender;

    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool click_buy = false;

    bool handle_keydown(const SDL_Event& event);
    bool handle_keyup(const SDL_Event& event);
    bool handle_click(const SDL_Event& event, int time);
    bool handle_shop_movement(const SDL_Event& event, int time);

    bool try_move(bool& movement, MovementDTO move);
    bool try_stop(bool& movement, MovementDTO move);

public:
    explicit InputManager(SDLManager& sdl, ClientProtocol& protocol);
    bool handle_waiting();
    bool handle_movement();
    bool handle_buy_events(int time);
    bool handle_countdown();
    bool handle_leaderboard();
    bool handle_game_finished();
    bool is_moving() const;
    void handle_disconnect();
    ~InputManager();
};


#endif
