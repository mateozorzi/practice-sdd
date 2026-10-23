#include "power_up_dto.h"


PowerUpDTO::PowerUpDTO(const uint8_t id, const PowerUpType type, const uint32_t x_position,
                       const uint32_t y_position, const bool active, const uint32_t collected_by):
        id(id),
        type(type),
        x_position(x_position),
        y_position(y_position),
        active(active),
        collected_by(collected_by) {}


PowerUpEffectDTO::PowerUpEffectDTO(const uint32_t player_id, const PowerUpType type,
                                   const uint32_t remaining_ms):
        player_id(player_id), type(type), remaining_ms(remaining_ms) {}
