#ifndef WORLD_SNAPSHOT_DTO_H
#define WORLD_SNAPSHOT_DTO_H

#include <vector>

#include "../common/car_position_dto.h"


struct WorldSnapshotDTO {

    std::vector<CarPositionDTO> car_positions;


public:
    // WorldSnapshot(const WorldSnapshot&) = delete;
    // WorldSnapshot& operator=(const WorldSnapshot&) = delete;

    WorldSnapshotDTO() = default;

    WorldSnapshotDTO(const WorldSnapshotDTO& ws);

    void add_car_position(const CarPositionDTO& car_position);
    const std::vector<CarPositionDTO>& get_car_positions() const;
    void reserve(size_t count);
    size_t get_size() const;
};


#endif
