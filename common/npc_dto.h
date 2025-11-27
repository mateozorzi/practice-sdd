#ifndef NPC_DTO_H
#define NPC_DTO_H

#include <cstdint>

struct NpcDTO {
    uint32_t npc_id;
    uint32_t x_position;
    uint32_t y_position;
    uint32_t angle;
    
    NpcDTO(uint32_t npc_id, uint32_t x_position, uint32_t y_position, uint32_t angle):
        npc_id(npc_id),
        x_position(x_position),
        y_position(y_position),
        angle(angle) {}
    
    NpcDTO():
        npc_id(0),
        x_position(0),
        y_position(0),
        angle(0) {}
};

#endif  // NPC_DTO_H
