#include "npc_dto.h"


NpcDTO::NpcDTO(const uint32_t npc_id, const uint32_t x_position, const uint32_t y_position,
               const uint32_t angle):
        npc_id(npc_id), x_position(x_position), y_position(y_position), angle(angle) {}
