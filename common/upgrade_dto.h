#ifndef UPGRADE_DTO_H
#define UPGRADE_DTO_H

#include <cstdint>

struct UpgradeDTO {
    uint8_t handling_increase;
    uint8_t speed_increase;
    uint8_t acceleration_increase;
    uint16_t health_increase;

    UpgradeDTO(uint8_t handling_increase, uint8_t speed_increase,
               uint8_t acceleration_increase, uint16_t health_increase) :
            handling_increase(handling_increase),
            speed_increase(speed_increase),
            acceleration_increase(acceleration_increase),
            health_increase(health_increase) {}


    //default 
    UpgradeDTO() :
            handling_increase(0),
            speed_increase(0),
            acceleration_increase(0),
            health_increase(0) {}
};

#endif