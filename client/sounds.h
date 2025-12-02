#ifndef SOUNDS_H
#define SOUNDS_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "resource_manager.h"

struct EngineState {
    Uint32 last_engine_time = 2500;
};

class Sounds {
private:
    SDL2pp::Mixer& mixer;
    ResourceManager& resources;
    std::unordered_map<uint32_t, int> crash_channels;
    int engine_channel = 0;
    int upgrade_channel = 0;
    EngineState engine_state;
    Uint32 engine_delay = 2500;

public:
    Sounds(SDL2pp::Mixer& mixer, ResourceManager& resources);

    void initialize_channels(const std::vector<uint32_t>& ids);

    void play_engine_sound(bool is_moving);
    void play_upgrade_sound();
    void play_crash_sound(uint32_t id);
};


#endif
