#include "player_dto.h"

#include "car_dto.h"


PlayerDTO::PlayerDTO(const std::string& username, const uint32_t id, const car_dto& car):
        username(username), id(id), car(car) {}

PlayerDTO::PlayerDTO(const std::string& username): username(username), id(0), car(car_dto()) {}

PlayerDTO::PlayerDTO(): username(""), id(0), car(car_dto()) {}
