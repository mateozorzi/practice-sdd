#ifndef RACE_H
#define RACE_H

#include <string>
#include <vector>
#include <cstdint>
#include "checkpoint_dto.h"

struct RaceDTO {

    std::string name;    
    std::string city;
    std::vector<CheckpointDTO> checkpoints;
    uint8_t size_checkpoints;
    uint8_t laps;

    RaceDTO(const std::string& name,
            const std::string& city,
            const std::vector<CheckpointDTO>& checkpoints,
            uint8_t laps) :
            name(name),
            city(city),
            checkpoints(checkpoints),
            size_checkpoints(static_cast<uint8_t>(checkpoints.size())),
            laps(laps) {}

    RaceDTO() :
            name(""),
            city(""),
            checkpoints({}),
            size_checkpoints(0),
            laps(0) {}

    



};

#endif