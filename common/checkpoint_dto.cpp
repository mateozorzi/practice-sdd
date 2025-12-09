#include "checkpoint_dto.h"


CheckpointDTO::CheckpointDTO(const uint8_t id, const char orientation, const uint16_t x,
                             const uint16_t y, const uint8_t width, const uint8_t height,
                             const bool is_finish_line):
        id(id),
        orientation(orientation),
        x(x),
        y(y),
        width(width),
        height(height),
        is_finish_line(is_finish_line) {}
