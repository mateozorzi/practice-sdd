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
    bool finish_race;


    explicit PlayerDTO(const std::string& username = "", uint32_t id = 0,
                       const CarDTO& car = CarDTO(), uint16_t health = 0, bool dead = false,
                       uint16_t max_health = 0, bool finish_race = false);
};


#endif
