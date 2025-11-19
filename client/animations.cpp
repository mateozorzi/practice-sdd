#include "animations.h"

Animations::Animations(SDL2pp::Renderer& renderer, Camera& camera, ResourceManager& resources) : renderer(renderer), camera(camera),
    resources(resources) {}

void Animations::start_explosion(int current_it, SDL2pp::Point position) {
    if (!explosion.has_value()) {
        explosion = ExplosionAnimation{current_it, position};
    }
}

bool Animations::is_explosion_active(int current_it) {
    if (!explosion.has_value()) {
        return false;
    }

    int diff = current_it - explosion.value().start_it;
    if (diff >= explosion.value().duration) {
        explosion.reset();
        return false;
    }
    return true;
}

void Animations::render_explosion(int current_it) {
    if (!explosion.has_value()) {
        std::cout << "Hola" << std::endl;
        return;
    }

    ExplosionAnimation& animation = explosion.value();

    int frame = current_it - animation.start_it;
    if (frame >= animation.duration) {
        return;
    }

    SDL2pp::Texture& sheet = resources.get_texture("client/assets/sprites/explosion.png");

    int sprite_index = frame / animation.frame_duration;
    if (sprite_index >= animation.frames) {
        sprite_index = animation.frames - 1;
    }

    int src_x = (sprite_index % animation.rows_cols) * animation.frame_size;
    int src_y = (sprite_index / animation.rows_cols) * animation.frame_size;

    SDL2pp::Rect src(src_x, src_y, animation.frame_size, animation.frame_size);

    SDL2pp::Rect view = camera.get_view_rect();

    int screen_x = (animation.position.GetX() - view.x) * 1920 / view.w;
    int screen_y = (animation.position.GetY() - view.y) * 1920 / view.h;

    SDL2pp::Rect dst(screen_x - animation.scale_size / 2, screen_y - animation.scale_size, animation.scale_size, animation.scale_size);
    if (!camera.on_screen(dst)) return;

    SDL2pp::Rect explosion_world(
    animation.position.GetX() - animation.frame_size/2,
    animation.position.GetY() - animation.frame_size/2,
    animation.frame_size,
    animation.frame_size
    );

    if (!camera.on_screen(explosion_world))
        return;

    renderer.Copy(sheet, src, dst);
}






