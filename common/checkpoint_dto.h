#ifndef CHECKPOINT_DTO_H
#define CHECKPOINT_DTO_H

#include <cstdint>

struct CheckpointDTO {
    uint8_t id;
    char orientation;
    uint16_t x;
    uint16_t y;
    uint8_t width;
    uint8_t height;

    CheckpointDTO(const uint8_t id,
                  const char orientation,
                  const uint16_t x,
                  const uint16_t y,
                  const uint8_t width,
                  const uint8_t height) :
            id(id),
            orientation(orientation),
            x(x),
            y(y),
            width(width),
            height(height) {}
    

};

#endif