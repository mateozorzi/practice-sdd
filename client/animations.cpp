#include "animations.h"

#include "client_constants.h"

Animations::Animations(SDL2pp::Renderer& renderer, Camera& camera, ResourceManager& resources):
        renderer(renderer), camera(camera), resources(resources) {}

void Animations::start_explosion(uint32_t id, int current_it, const SDL2pp::Point& position) {
    if (explosions.find(id) == explosions.end()) {
        explosions.try_emplace(id, ExplosionAnimation{current_it, position});
    }
}

bool Animations::is_explosion_active(uint32_t id) const {
    return explosions.find(id) != explosions.end();
}

void Animations::render_explosions(int current_it) {
    if (explosions.empty()) {
        return;
    }

    SDL2pp::Texture& sprite = resources.get_texture("/var/nfs/assets/sprites/explosion.png");
    SDL2pp::Rect view = camera.get_view_rect();

    for (auto& [id, animation]: explosions) {
        int frame = current_it - animation.start_it;
        if (frame >= animation.duration)
            continue;

        int sprite_index = frame / animation.frame_duration;
        if (sprite_index >= animation.frames) {
            sprite_index = animation.frames - 1;
        }

        int src_x = (sprite_index % animation.rows_cols) * animation.frame_size;
        int src_y = (sprite_index / animation.rows_cols) * animation.frame_size;

        SDL2pp::Rect src(src_x, src_y, animation.frame_size, animation.frame_size);

        int screen_x = (animation.position.GetX() - view.x) * SCREEN_WIDTH / view.w;
        int screen_y = (animation.position.GetY() - view.y) * SCREEN_HEIGHT / view.h;

        SDL2pp::Rect dst(screen_x - animation.scale_size / 2, screen_y - animation.scale_size / 2,
                         animation.scale_size, animation.scale_size);

        SDL2pp::Rect explosion_world(animation.position.GetX() - animation.frame_size / 2,
                                     animation.position.GetY() - animation.frame_size / 2,
                                     animation.frame_size, animation.frame_size);

        if (!camera.on_screen(explosion_world))
            continue;

        renderer.Copy(sprite, src, dst);
    }
}

void Animations::clean_explosions(const int current_it) {
    for (auto it = explosions.begin(); it != explosions.end();) {
        int diff = current_it - it->second.start_it;
        if (diff >= it->second.duration) {
            it = explosions.erase(it);
        } else {
            ++it;
        }
    }
}

void Animations::start_camera_shake(int current_it) {
    if (!camera_shake.has_value()) {
        camera_shake = CameraShakeAnimation{current_it};
    }
}

bool Animations::is_camera_shake_active(int current_it) {
    if (!camera_shake.has_value()) {
        return false;
    }
    int diff = current_it - camera_shake.value().start_it;
    if (diff >= camera_shake.value().duration) {
        camera_shake.reset();
        return false;
    }
    return true;
}

SDL2pp::Point Animations::get_camera_shake() {
    if (!camera_shake.has_value()) {
        return SDL2pp::Point(0, 0);
    }

    int magnitude = 3;
    int dx = (std::rand() % (2 * magnitude + 1)) - magnitude;
    int dy = (std::rand() % (2 * magnitude + 1)) - magnitude;

    return SDL2pp::Point(dx, dy);
}

void Animations::restart_animations() {
    explosions.clear();
    camera_shake.reset();
}
