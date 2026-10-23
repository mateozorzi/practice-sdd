#ifndef WORLD_SNAPSHOT_DTO_H
#define WORLD_SNAPSHOT_DTO_H


#include <utility>
#include <vector>

#include "../common/car_position_dto.h"
#include "../common/npc_dto.h"
#include "../common/player_dto.h"
#include "../common/power_up_dto.h"


struct WorldSnapshotDTO {

    std::vector<std::pair<PlayerDTO, CarPositionDTO>> car_positions;
    std::vector<NpcDTO> npc_positions;
    std::vector<PowerUpDTO> power_ups;
    std::vector<PowerUpEffectDTO> effects;
    uint8_t size;
    uint8_t npc_size;


    explicit WorldSnapshotDTO(
            const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& car_positions = {});

    void add_car_position(const PlayerDTO& player, const CarPositionDTO& car_position);
    void add_npc_position(const NpcDTO& npc_position);
    void add_power_up(const PowerUpDTO& power_up);
    void add_effect(const PowerUpEffectDTO& effect);
};


#endif
