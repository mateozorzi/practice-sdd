#include "world_snapshot_dto.h"
#include <iostream>

WorldSnapshotDTO::WorldSnapshotDTO(const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& car_positions)
    : car_positions(car_positions), size(car_positions.size()) {}

WorldSnapshotDTO::WorldSnapshotDTO() : car_positions(), size(0) {}

void WorldSnapshotDTO::add_car_position(const PlayerDTO& player, const CarPositionDTO& car_position) {
    car_positions.push_back(std::make_pair(player, car_position));
    size++;
}

const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& WorldSnapshotDTO::get_car_positions() const {

    return car_positions;
}

void WorldSnapshotDTO::reserve(const size_t count) { car_positions.reserve(count); }


size_t WorldSnapshotDTO::get_size() const { return size; }
