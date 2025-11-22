#include "player_dto.h"

#include "car_dto.h"


PlayerDTO::PlayerDTO(const std::string& username, const uint32_t id, const CarDTO& car,
                     uint16_t health, bool dead):
        username(username), id(id), car(car), health(health), dead(dead) {}

PlayerDTO::PlayerDTO(const std::string& username):
        username(username), id(0), car(CarDTO()), health(-1), dead(false) {}

PlayerDTO::PlayerDTO(): username(""), id(0), car(CarDTO()), health(-1), dead(false) {}
