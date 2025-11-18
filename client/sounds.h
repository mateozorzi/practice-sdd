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
    int player_engine_channel = 0;
    EngineState engine_state;
    Uint32 engine_delay = 2500;

public:
    Sounds(SDL2pp::Mixer& mixer, ResourceManager& resources);

    void initialize_channels();

    void play_engine_sound(bool is_moving);
};


#endif
