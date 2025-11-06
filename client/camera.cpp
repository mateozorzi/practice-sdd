#include "camera.h"

Camera::Camera(const int view_w, const int view_h, const int world_w, const int world_h):
        camera_x(0),
        camera_y(0),
        view_width(view_w),
        view_height(view_h),
        world_width(world_w),
        world_height(world_h) {}

void Camera::update(const int player_x, const int player_y) {
    camera_x = player_x - (view_width / 2) + 25;
    camera_y = player_y - (view_height / 2) + 25;

    // Limite del mundo
    if (camera_x < 0)
        camera_x = 0;
    if (camera_y < 0)
        camera_y = 0;
    if (camera_x > world_width - view_width)
        camera_x = world_width - view_width;
    if (camera_y > world_height - view_height)
        camera_y = world_height - view_height;
}

SDL2pp::Rect Camera::get_view_rect() const { return {camera_x, camera_y, view_width, view_height}; }
