#ifndef PLAYER_DTO_H
#define PLAYER_DTO_H


#include <cstdint>
#include <string>

#include "car_dto.h"


struct PlayerDTO {


    std::string username;
    uint32_t id;
    CarDTO car;

    PlayerDTO(const std::string& username, const uint32_t id, const CarDTO& car);
    explicit PlayerDTO(const std::string& username);
    PlayerDTO();
};


#endif
