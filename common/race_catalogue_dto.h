#ifndef RACE_CATALOGUE_DTO_H
#define RACE_CATALOGUE_DTO_H


#include <cstdint>
#include <vector>

#include "race_dto.h"


struct RaceCatalogueDTO {


    uint8_t size;
    std::vector<RaceDTO> races;


    explicit RaceCatalogueDTO(const std::vector<RaceDTO>& races = {});
};


#endif
