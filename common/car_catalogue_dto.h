#ifndef CAR_CATALOGUE_DTO
#define CAR_CATALOGUE_DTO


#include <vector>

#include "car_dto.h"


struct CarCatalogueDTO {


    const std::vector<car_dto> cars;
    const uint8_t size;

    explicit CarCatalogueDTO(std::vector<car_dto> cars = {});
};


#endif
