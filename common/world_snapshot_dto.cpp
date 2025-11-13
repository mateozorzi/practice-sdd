#include "world_snapshot_dto.h"

WorldSnapshotDTO::WorldSnapshotDTO(const WorldSnapshotDTO& ws) : car_positions(ws.car_positions) {}

void WorldSnapshotDTO::add_car_position(const CarPositionDTO& car_position) {

    car_positions.push_back(car_position);
}

const std::vector<CarPositionDTO>& WorldSnapshotDTO::get_car_positions() const {

    return car_positions;
}

void WorldSnapshotDTO::reserve(const size_t count) { car_positions.reserve(count); }


size_t WorldSnapshotDTO::get_size() const { return car_positions.size(); }
