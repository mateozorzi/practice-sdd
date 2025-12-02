#include "sounds.h"

#include <SDL2/SDL_timer.h>

#include "SDL2pp/Chunk.hh"

Sounds::Sounds(SDL2pp::Mixer& mixer, ResourceManager& resources):
        mixer(mixer), resources(resources) {}

void Sounds::initialize_channels(const std::vector<uint32_t>& ids) {
    int current_channel = 1;

    for (const auto& id : ids) crash_channels[id] = current_channel++;

    engine_channel = current_channel++;
    upgrade_channel = current_channel++;
}

void Sounds::play_engine_sound(const bool is_moving) {
    if (!is_moving)
        return;

    const Uint32 now = SDL_GetTicks();
    auto& state = engine_state;

    if (now - state.last_engine_time < engine_delay)
        return;

    SDL2pp::Chunk& engine_sound = resources.get_sound("client/assets/sounds/sfx_car_engine.wav");
    mixer.PlayChannel(engine_channel, engine_sound);

    state.last_engine_time = now;
}

void Sounds::play_upgrade_sound() {
    SDL2pp::Chunk& upgrade_sound = resources.get_sound("client/assets/sounds/sfx_upgrade.wav");
    mixer.PlayChannel(upgrade_channel, upgrade_sound);
}

void Sounds::play_crash_sound(const uint32_t id) {
    SDL2pp::Chunk& crash_sound = resources.get_sound("client/assets/sounds/crash.wav");
    auto it = crash_channels.find(id);
    if (it == crash_channels.end() || it->second == -1) return;

    mixer.PlayChannel(it->second, crash_sound);
}
