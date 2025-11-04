#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2pp/SDL2pp.hh>

class Camera {
private:
    int camera_x;
    int camera_y;
    int view_width;
    int view_height;
    int world_width;
    int world_height;

public:
    Camera(int view_w, int view_h, int world_w, int world_h);

    void update(int player_x, int player_y);

    SDL2pp::Rect get_view_rect() const;
};

#endif  // CAMERA_H
