#include "car_catalogue_dto.h"


CarCatalogueDTO::CarCatalogueDTO(std::vector<car_dto> cars):
        cars(std::move(cars)), size(static_cast<uint8_t>(this->cars.size())) {}
