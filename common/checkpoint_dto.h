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
    bool is_finish_line = false;

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
    
    CheckpointDTO(const uint8_t id,
                  const char orientation,
                  const uint16_t x,
                  const uint16_t y,
                  const uint8_t width,
                  const uint8_t height, const bool is_finish_line) :
            id(id),
            orientation(orientation),
            x(x),
            y(y),
            width(width),
            height(height), is_finish_line(is_finish_line) {}
    
    CheckpointDTO() :
            id(0),
            orientation('H'),
            x(0),
            y(0),
            width(0),
            height(0),
            is_finish_line(false) {}

};

#endif