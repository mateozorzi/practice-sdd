#ifndef PLAYER_DTO_H
#define PLAYER_DTO_H


#include <cstdint>
#include <string>

#include "car_dto.h"


struct PlayerDTO {


    std::string username;
    uint32_t id;
    CarDTO car;
    uint16_t health;
    bool dead;
    uint16_t max_health;

    PlayerDTO(const std::string& username, const uint32_t id, const CarDTO& car, uint16_t health,
              bool dead, uint16_t max_health);
    explicit PlayerDTO(const std::string& username);
    PlayerDTO();
};


#endif
