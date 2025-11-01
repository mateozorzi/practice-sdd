#include "camera.h"

// NOLINTNEXTLINE(build/namespaces)
using namespace SDL2pp;

Camera::Camera(int view_w, int view_h, int world_w, int world_h):
        camera_x(0),
        camera_y(0),
        view_width(view_w),
        view_height(view_h),
        world_width(world_w),
        world_height(world_h) {}

void Camera::update(int player_x, int player_y) {
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

Rect Camera::get_view_rect() const { return Rect(camera_x, camera_y, view_width, view_height); }

int Camera::get_camera_x() const { return camera_x; }

int Camera::get_camera_y() const { return camera_y; }

int Camera::get_view_width() const { return view_width; }

int Camera::get_view_height() const { return view_height; }
