#ifndef ANIMATIONS_H
#define ANIMATIONS_H
#include <optional>
#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "camera.h"
#include "resource_manager.h"

class Animations {
private:
    SDL2pp::Renderer& renderer;
    Camera& camera;
    ResourceManager& resources;

    struct ExplosionAnimation {
        int start_it;
        SDL2pp::Point position;
        int frame_size = 60;
        int scale_size = 240;
        int rows_cols = 4;
        int frames = 16;
        int frame_duration = 2;
        int duration = frames * frame_duration;
    };

    std::unordered_map<uint32_t, ExplosionAnimation> explosions;

    struct CameraShakeAnimation {
        int start_it;
        int duration = 30;
    };

    std::optional<CameraShakeAnimation> camera_shake;

public:
    Animations(SDL2pp::Renderer& renderer, Camera& camera, ResourceManager& resources);

    void start_explosion(uint32_t id, int current_it, SDL2pp::Point position);
    bool is_explosion_active(uint32_t id) const;
    void render_explosions(int current_it);
    void clean_explosions(int current_it);

    void start_camera_shake(int current_it);
    bool is_camera_shake_active(int current_it);
    SDL2pp::Point get_camera_shake();

    void restart_animations();
};


#endif
