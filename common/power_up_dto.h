#ifndef POWER_UP_DTO_H
#define POWER_UP_DTO_H


#include <cstdint>

#include "power_up_type.h"


struct PowerUpDTO {


    uint8_t id;
    PowerUpType type;
    uint32_t x_position;  // pixels, center of the power-up
    uint32_t y_position;
    bool active;


    explicit PowerUpDTO(uint8_t id = 0, PowerUpType type = PowerUpType::Nitro,
                        uint32_t x_position = 0, uint32_t y_position = 0, bool active = false);

    bool operator==(const PowerUpDTO& other) const = default;
};


// Effect with duration (Nitro or Shield) that a player has active
struct PowerUpEffectDTO {


    uint32_t player_id;
    PowerUpType type;
    uint32_t remaining_ms;


    explicit PowerUpEffectDTO(uint32_t player_id = 0, PowerUpType type = PowerUpType::Nitro,
                              uint32_t remaining_ms = 0);

    bool operator==(const PowerUpEffectDTO& other) const = default;
};


#endif
