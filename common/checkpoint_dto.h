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
    bool is_finish_line;

    explicit CheckpointDTO(uint8_t id = 0, char orientation = 'H', uint16_t x = 0, uint16_t y = 0,
                           uint8_t width = 0, uint8_t height = 0, bool is_finish_line = false);
};


#endif
