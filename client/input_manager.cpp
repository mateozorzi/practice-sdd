#include "input_manager.h"

InputManager::InputManager(ClientProtocol& protocol): sender(protocol) { sender.start(); }

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

bool InputManager::handle_movement() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                return false;
            }
            handle_keydown(event);
        } else if (event.type == SDL_KEYUP) {
            handle_keyup(event);
        }
    }
    return true;
}

InputManager::~InputManager() {
    if (sender.is_alive()) {
        sender.stop();
        sender.close_queue();
        sender.join();
    }
}
