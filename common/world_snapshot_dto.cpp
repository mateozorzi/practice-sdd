#include "world_snapshot_dto.h"

#include <iostream>

WorldSnapshotDTO::WorldSnapshotDTO(
        const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& car_positions):
        car_positions(car_positions), npc_positions(), size(car_positions.size()), npc_size(0) {}

WorldSnapshotDTO::WorldSnapshotDTO(): car_positions(), npc_positions(), size(0), npc_size(0) {}

void WorldSnapshotDTO::add_car_position(const PlayerDTO& player,
                                        const CarPositionDTO& car_position) {
    car_positions.push_back(std::make_pair(player, car_position));
    size++;
}

void WorldSnapshotDTO::add_npc_position(const NpcDTO& npc_position) {
    npc_positions.push_back(npc_position);
    npc_size++;
}

const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& WorldSnapshotDTO::get_car_positions()
        const {

    return car_positions;
}

const std::vector<NpcDTO>& WorldSnapshotDTO::get_npc_positions() const {
    return npc_positions;
}

void WorldSnapshotDTO::reserve(const size_t count) { car_positions.reserve(count); }


size_t WorldSnapshotDTO::get_size() const { return size; }

size_t WorldSnapshotDTO::get_npc_size() const { return npc_size; }
