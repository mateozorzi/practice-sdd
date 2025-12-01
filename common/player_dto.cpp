#include "player_dto.h"

#include "car_dto.h"


PlayerDTO::PlayerDTO(const std::string& username, const uint32_t id, const CarDTO& car,
                     uint16_t health, bool dead, uint16_t max_health, bool finish_race):
        username(username),
        id(id),
        car(car),
        health(health),
        dead(dead),
        max_health(max_health),
        finish_race(finish_race) {}

PlayerDTO::PlayerDTO(const std::string& username):
        username(username),
        id(-1),
        car(CarDTO()),
        health(-1),
        dead(false),
        max_health(0),
        finish_race(false) {}

PlayerDTO::PlayerDTO():
        username(""),
        id(-1),
        car(CarDTO()),
        health(-1),
        dead(false),
        max_health(0),
        finish_race(false) {}
