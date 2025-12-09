#include "player_dto.h"


PlayerDTO::PlayerDTO(const std::string& username, const uint32_t id, const CarDTO& car,
                     const uint16_t health, const bool dead, const uint16_t max_health,
                     const bool finish_race):
        username(username),
        id(id),
        car(car),
        health(health),
        dead(dead),
        max_health(max_health),
        finish_race(finish_race) {}
