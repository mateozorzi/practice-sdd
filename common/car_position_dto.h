#ifndef CAR_POSITION_DTO
#define CAR_POSITION_DTO


#include <cstdint>

#include "checkpoint_dto.h"


enum class CarLayer : uint8_t {

    ON_GROUND = 0,
    ON_BRIDGE = 1,
    UNDER_BRIDGE = 2
};

struct CarPositionDTO {

    uint32_t client_id;
    uint32_t x_position;
    uint32_t y_position;
    uint32_t angle;
    CheckpointDTO next;
    CarLayer layer;
    uint8_t current_lap;
    uint8_t race_lap;


    explicit CarPositionDTO(uint32_t client_id = 0, uint32_t x_position = 0,
                            uint32_t y_position = 0, uint32_t angle = 0,
                            const CheckpointDTO& next = CheckpointDTO(),
                            const CarLayer layer = CarLayer::ON_GROUND, uint8_t current_lap = 0,
                            uint8_t race_lap = 0);
};


#endif
