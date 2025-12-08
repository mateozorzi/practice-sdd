#ifndef RACE_H
#define RACE_H


#include <cstdint>
#include <string>
#include <vector>

#include "checkpoint_dto.h"


struct RaceDTO {

    std::string name;
    std::string city;
    std::vector<CheckpointDTO> checkpoints;
    uint8_t size_checkpoints;
    uint8_t laps;
    std::string bridges_data;
    std::string decorations_data;

    RaceDTO(const std::string& name, const std::string& city,
            const std::vector<CheckpointDTO>& checkpoints, uint8_t laps,
            const std::string& bridges_data, const std::string& decorations_data):
            name(name),
            city(city),
            checkpoints(checkpoints),
            size_checkpoints(static_cast<uint8_t>(checkpoints.size())),
            laps(laps),
            bridges_data(bridges_data),
            decorations_data(decorations_data) {}

    RaceDTO():
            name(""),
            city(""),
            checkpoints({}),
            size_checkpoints(0),
            laps(0),
            bridges_data("[]"),
            decorations_data("[]") {}
};


#endif
