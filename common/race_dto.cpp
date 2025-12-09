#include "race_dto.h"


RaceDTO::RaceDTO(const std::string& name, const std::string& city,
                 const std::vector<CheckpointDTO>& checkpoints, const uint8_t laps,
                 const std::string& bridges_data, const std::string& decorations_data):
        name(name),
        city(city),
        size_checkpoints(static_cast<uint8_t>(checkpoints.size())),
        checkpoints(checkpoints),
        laps(laps),
        bridges_data(bridges_data),
        decorations_data(decorations_data) {}
