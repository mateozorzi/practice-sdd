#ifndef RACE_DTO_H
#define RACE_DTO_H


#include <cstdint>
#include <string>
#include <vector>

#include "checkpoint_dto.h"


struct RaceDTO {


    std::string name;
    std::string city;
    uint8_t size_checkpoints;
    std::vector<CheckpointDTO> checkpoints;
    uint8_t laps;
    std::string bridges_data;
    std::string decorations_data;


    explicit RaceDTO(const std::string& name = "", const std::string& city = "",
                     const std::vector<CheckpointDTO>& checkpoints = {}, uint8_t laps = 0,
                     const std::string& bridges_data = "[]",
                     const std::string& decorations_data = "[]");
};


#endif
