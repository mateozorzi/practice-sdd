#include "car_position_dto.h"


CarPositionDTO::CarPositionDTO(const uint32_t client_id, const uint32_t x_position,
                               const uint32_t y_position, const uint32_t angle):
        client_id(client_id), x_position(x_position), y_position(y_position), angle(angle) {}
