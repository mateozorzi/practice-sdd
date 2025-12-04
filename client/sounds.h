#ifndef SOUNDS_H
#define SOUNDS_H

#include <string>
#include <unordered_map>
#include <vector>

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
    std::unordered_map<uint32_t, int> explosion_channels;
    int current_channel = 1;
    int engine_channel = -1;
    int upgrade_channel = -1;
    int music_channel = -1;
    EngineState engine_state;
    Uint32 engine_delay = 2500;

    Uint8 calculate_distance(const SDL2pp::Point& event_pos, const SDL2pp::Point& target_pos);

public:
    Sounds(SDL2pp::Mixer& mixer, ResourceManager& resources);

    void initialize_channels(const std::vector<uint32_t>& ids);

    void play_engine_sound(bool is_moving);
    void play_upgrade_sound();
    void play_crash_sound(uint32_t id, const SDL2pp::Point& event_pos,
                          const SDL2pp::Point& target_pos);
    void play_explosion_sound(uint32_t id, const SDL2pp::Point& event_pos,
                              const SDL2pp::Point& target_pos);

    void play_background_music(const std::string& path, int loops = -1);
    void stop_background_music();
    void set_music_volume(int volume);
};


#endif
