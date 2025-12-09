#ifndef NPC_DTO_H
#define NPC_DTO_H


#include <cstdint>


struct NpcDTO {


    uint32_t npc_id;
    uint32_t x_position;
    uint32_t y_position;
    uint32_t angle;


    explicit NpcDTO(uint32_t npc_id = 0, uint32_t x_position = 0, uint32_t y_position = 0,
                    uint32_t angle = 0);
};


#endif
