#include "world_snapshot_dto.h"


WorldSnapshotDTO::WorldSnapshotDTO(
        const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& car_positions):
        car_positions(car_positions), size(car_positions.size()), npc_size(npc_positions.size()) {}


void WorldSnapshotDTO::add_car_position(const PlayerDTO& player,
                                        const CarPositionDTO& car_position) {

    car_positions.push_back(std::make_pair(player, car_position));
    size++;
}

void WorldSnapshotDTO::add_npc_position(const NpcDTO& npc_position) {

    npc_positions.push_back(npc_position);
    npc_size++;
}
