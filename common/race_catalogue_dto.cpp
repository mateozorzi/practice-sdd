#include "race_catalogue_dto.h"


RaceCatalogueDTO::RaceCatalogueDTO(const std::vector<RaceDTO>& races):
            size(static_cast<uint8_t>(races.size())), races(races) {}
