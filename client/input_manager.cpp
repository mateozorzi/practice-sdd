#include "input_manager.h"

InputManager::InputManager(SDLManager& sdl, ClientProtocol& protocol): sdl(sdl), sender(protocol) {
    sender.start();
}

bool InputManager::try_move(bool& movement, const MovementDTO move) {
    if (!movement) {
        sender.add_event(move);
        movement = true;
    }
    return true;
}

bool InputManager::try_stop(bool& movement, const MovementDTO move) {
    if (movement) {
        sender.add_event(move);
        movement = false;
    }
    return true;
}

bool InputManager::handle_keydown(const SDL_Event& event) {
    if (event.type != SDL_KEYDOWN) {
        return false;
    }

    switch (event.key.keysym.sym) {
        case SDLK_w:
            return try_move(w, MovementDTO{Direction::UP, true});
        case SDLK_a:
            return try_move(a, MovementDTO{Direction::LEFT, true});
        case SDLK_s:
            return try_move(s, MovementDTO{Direction::DOWN, true});
        case SDLK_d:
            return try_move(d, MovementDTO{Direction::RIGHT, true});
        default:
            return false;
    }
}

bool InputManager::handle_keyup(const SDL_Event& event) {
    if (event.type != SDL_KEYUP) {
        return false;
    }

    switch (event.key.keysym.sym) {
        case SDLK_w:
            return try_stop(w, MovementDTO{Direction::UP, false});
        case SDLK_a:
            return try_stop(a, MovementDTO{Direction::LEFT, false});
        case SDLK_s:
            return try_stop(s, MovementDTO{Direction::DOWN, false});
        case SDLK_d:
            return try_stop(d, MovementDTO{Direction::RIGHT, false});
        default:
            return false;
    }
}

bool InputManager::handle_click(const SDL_Event& event, int time) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !click_buy) {
        click_buy = true;

        int mouse_x = event.button.x;
        int mouse_y = event.button.y;

        auto opt_button = sdl.interact_button(mouse_x, mouse_y, time);
        if (opt_button.has_value()) {
            ButtonType button = opt_button.value();
            sdl.play_upgrade_sound();
            switch (button) {
                case Acceleration:
                    sender.add_event(BuyUpgradeDTO{Acceleration});
                    break;
                case Handling:
                    sender.add_event(BuyUpgradeDTO{Handling});
                    break;
                case Health:
                    sender.add_event(BuyUpgradeDTO{Health});
                    break;
                case Speed:
                    sender.add_event(BuyUpgradeDTO{Speed});
                    break;
                default:
                    break;
            }
        }
        return true;
    }
    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && click_buy) {
        click_buy = false;
        return true;
    }
    return false;
}

bool InputManager::handle_waiting() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                handle_disconnect();
                return false;
            }
        }
    }
    return true;
}

bool InputManager::handle_shop_movement(const SDL_Event& event, int time) {
    if (event.type == SDL_MOUSEMOTION) {
        int mouse_x = event.motion.x;
        int mouse_y = event.motion.y;

        sdl.interact_button(mouse_x, mouse_y, time);
        return true;
    }
    return false;
}

bool InputManager::handle_buy_events(int time) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return false;
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                handle_disconnect();
                return false;
            }
        }
        if (handle_click(event, time))
            continue;
        if (handle_shop_movement(event, time))
            continue;
    }
    return true;
}

bool InputManager::handle_countdown() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        // cheats
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                handle_disconnect();
                return false;
            }
        }
    }
    return true;
}

bool InputManager::handle_leaderboard() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                handle_disconnect();
                return false;
            }
        }
    }
    return true;
}

bool InputManager::handle_game_finished() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                handle_disconnect();
                return false;
            }
        }
    }
    return true;
}

bool InputManager::handle_movement() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                handle_disconnect();
                return false;
            } else if (handle_cheats(event)) {
                return true;
            }
            handle_keydown(event);
        } else if (event.type == SDL_KEYUP) {
            handle_keyup(event);
        }
    }
    return true;
}

bool InputManager::is_moving() const { return w || a || s || d; }

void InputManager::handle_disconnect() {
    sender.add_event(ClientActionDTO(ClientAction::DISCONNECT));
}

void InputManager::handle_cheat_race_winner() {
    sender.add_event(ClientActionDTO(ClientAction::CHEAT_WIN_RACE));
}

void InputManager::handle_cheat_game_winner() {
    sender.add_event(ClientActionDTO(ClientAction::CHEAT_WIN_GAME));
}

void InputManager::handle_infinite_health() {
    sender.add_event(ClientActionDTO(ClientAction::CHEAT_INFINITE_HEALTH));
}

bool InputManager::handle_cheats(const SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_1) {
        //win the race cheat
        handle_cheat_race_winner();
        return true;
    } else if (event.key.keysym.sym == SDLK_2) {
        // infinite health cheat
        handle_infinite_health();
        return true;
    } else if (event.key.keysym.sym == SDLK_3) {
        // game winner cheat
        handle_cheat_game_winner();
        return true;
    }
    return false;
}


InputManager::~InputManager() {
    if (sender.is_alive()) {
        sender.stop();
        sender.close_queue();
        sender.join();
    }
}
