#ifndef WORLD_SNAPSHOT_DTO_H
#define WORLD_SNAPSHOT_DTO_H

#include <vector>

#include "../common/car_position_dto.h"
#include "../common/player_dto.h"


struct WorldSnapshotDTO {

    public:
    std::vector<std::pair<PlayerDTO, CarPositionDTO>> car_positions;
    uint8_t size;
    // WorldSnapshot(const WorldSnapshot&) = delete;
    // WorldSnapshot& operator=(const WorldSnapshot&) = delete;

    WorldSnapshotDTO();

    WorldSnapshotDTO(const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& car_positions);

    void add_car_position(const PlayerDTO& player, const CarPositionDTO& car_position);
    const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& get_car_positions() const;
    void reserve(size_t count);
    size_t get_size() const;
};


#endif
