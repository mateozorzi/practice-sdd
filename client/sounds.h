#ifndef SOUNDS_H
#define SOUNDS_H
#include <SDL2pp/Chunk.hh>

#include "SDL2pp/Mixer.hh"
#include "cmake-build-debug/_deps/sdl2-src/include/SDL_stdinc.h"

struct EngineState {
    Uint32 last_engine_time = 2500;
};

class Sounds {
private:
    SDL2pp::Mixer& mixer;
    int player_engine_channel = 0;
    EngineState engine_state;
    Uint32 engine_delay = 2500;

    SDL2pp::Chunk engine_sound;

public:
    explicit Sounds(SDL2pp::Mixer& mixer);

    void initialize_channels();

    void play_engine_sound(bool is_moving);
};


#endif
