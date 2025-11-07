#ifndef CAR_POSITION_DTO
#define CAR_POSITION_DTO


#include <cstdint>


struct CarPositionDTO {


    uint32_t client_id;
    uint32_t x_position;
    uint32_t y_position;
    uint32_t angle;


    explicit CarPositionDTO(uint32_t client_id = 0, uint32_t x_position = 0, uint32_t y_position = 0, uint32_t angle = 0);
};


#endif
