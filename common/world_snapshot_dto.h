#ifndef WORLD_SNAPSHOT_DTO_H
#define WORLD_SNAPSHOT_DTO_H

#include <utility>
#include <vector>

#include "../common/car_position_dto.h"
#include "../common/npc_dto.h"
#include "../common/player_dto.h"


struct WorldSnapshotDTO {

public:
    std::vector<std::pair<PlayerDTO, CarPositionDTO>> car_positions;
    std::vector<NpcDTO> npc_positions;
    uint8_t size;
    uint8_t npc_size;
    // WorldSnapshot(const WorldSnapshot&) = delete;
    // WorldSnapshot& operator=(const WorldSnapshot&) = delete;

    WorldSnapshotDTO();

    explicit WorldSnapshotDTO(
            const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& car_positions);

    void add_car_position(const PlayerDTO& player, const CarPositionDTO& car_position);
    void add_npc_position(const NpcDTO& npc_position);
    const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& get_car_positions() const;
    const std::vector<NpcDTO>& get_npc_positions() const;
    void reserve(size_t count);
    size_t get_size() const;
    size_t get_npc_size() const;
};


#endif
