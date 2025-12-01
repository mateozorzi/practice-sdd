#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H


#include <SDL2/SDL_events.h>

#include "sdl_manager.h"
#include "sender.h"
#include "upgrade_shop.h"


class InputManager {

    SDLManager& sdl;
    Sender sender;

    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool click_buy = false;
    bool player_disconnected = false;

    void try_move(bool& movement, MovementDTO move);
    void try_stop(bool& movement, MovementDTO move);

    void handle_keydown(const SDL_Event& event);
    void handle_keyup(const SDL_Event& event);
    void handle_click(const SDL_Event& event, int time);
    void handle_shop_movement(const SDL_Event& event, int time);


public:
    explicit InputManager(SDLManager& sdl, ClientProtocol& protocol);
    void handle_waiting();
    void handle_buy_events(int time);
    void handle_countdown();
    void handle_movement();
    void handle_leaderboard();
    void handle_game_finished();
    bool is_moving() const;
    void handle_disconnect();
    void handle_cheat_race_winner();
    void handle_cheat_game_winner();
    void handle_infinite_health();
    void handle_cheats(const SDL_Event& event);
    ~InputManager();
};


#endif
