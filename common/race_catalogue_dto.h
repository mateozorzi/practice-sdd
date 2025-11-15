#ifndef RACE_CATALOGUE_DTO_H
#define RACE_CATALOGUE_DTO_H

#include <cstdint>
#include <vector>
#include "race_dto.h"


struct RaceCatalogueDTO {
    uint8_t size;
    std::vector<RaceDTO> races;


    RaceCatalogueDTO() : size(0), races({}) {}

    RaceCatalogueDTO(const std::vector<RaceDTO>& races) :
            size(static_cast<uint8_t>(races.size())), races(races) {}

};

#endif
