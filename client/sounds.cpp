#include "sounds.h"

#include <complex>

#include <SDL2/SDL_timer.h>

#include "SDL2pp/Chunk.hh"

Sounds::Sounds(SDL2pp::Mixer& mixer, ResourceManager& resources):
        mixer(mixer), resources(resources) {}

void Sounds::initialize_channels() {
    int current_channel = 1;

    // Agregar channel para los demas jugadores, para choques y win

    player_engine_channel = current_channel;
}

void Sounds::play_engine_sound(const bool is_moving) {
    if (!is_moving)
        return;

    const Uint32 now = SDL_GetTicks();
    auto& state = engine_state;

    if (now - state.last_engine_time < engine_delay)
        return;

    if (player_engine_channel == -1) {
        return;
    }

    SDL2pp::Chunk& engine_sound = resources.get_sound("client/assets/sounds/sfx_car_engine.wav");
    mixer.PlayChannel(player_engine_channel, engine_sound);

    state.last_engine_time = now;
}

void Sounds::play_upgrade_sound() {
    const int upgrade_channel = 2;
    SDL2pp::Chunk& upgrade_sound = resources.get_sound("client/assets/sounds/sfx_upgrade.wav");
    mixer.PlayChannel(upgrade_channel, upgrade_sound);
}

void Sounds::play_crash_sound() {
    const int crash_channel = 3;
    SDL2pp::Chunk& crash_sound = resources.get_sound("client/assets/sounds/crash.wav");
    mixer.PlayChannel(crash_channel, crash_sound);
}
