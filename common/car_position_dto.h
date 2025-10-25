#ifndef CAR_POSITION_DTO
#define CAR_POSITION_DTO


#include <cstdint>


struct CarPositionDTO {


    const uint32_t x_position;
    const uint32_t y_position;
    const uint32_t angle;


    explicit CarPositionDTO(uint32_t x_position = 0, uint32_t y_position = 0, uint32_t angle = 0);
};


#endif
