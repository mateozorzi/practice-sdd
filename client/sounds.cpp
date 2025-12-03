#include "sounds.h"

#include <SDL2/SDL_timer.h>

#include "SDL2pp/Chunk.hh"

Uint8 Sounds::calculate_distance(const SDL2pp::Point &event_pos, const SDL2pp::Point &target_pos) {
    const float dx = static_cast<float>(event_pos.x - target_pos.x);
    const float dy = static_cast<float>(event_pos.y - target_pos.y);
    const float dist = std::sqrt(dx * dx + dy * dy);

    if (dist >= 255) {
        return 255;
    }

    const float ratio = dist / 100.0f;
    const Uint8 sdl_distance = static_cast<Uint8>(ratio * 255);

    return sdl_distance;
}

Sounds::Sounds(SDL2pp::Mixer& mixer, ResourceManager& resources):
        mixer(mixer), resources(resources) {}

void Sounds::initialize_channels(const std::vector<uint32_t>& ids) {
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

void Sounds::play_crash_sound(const uint32_t id, const SDL2pp::Point& event_pos, const SDL2pp::Point& target_pos) {
    const Uint8 sdl_distance = calculate_distance(event_pos, target_pos);

    if (sdl_distance >= 255) return;

    SDL2pp::Chunk& crash_sound = resources.get_sound("client/assets/sounds/crash.wav");
    auto it = crash_channels.find(id);
    if (it == crash_channels.end()) {
        int new_channel = current_channel++;
        crash_channels[id] = new_channel;
        it = crash_channels.find(id);
    }

    if (it->second == -1) return;

    mixer.PlayChannel(it->second, crash_sound);
    mixer.SetDistance(it->second, sdl_distance);
}
