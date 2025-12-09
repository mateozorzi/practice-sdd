#ifndef UPGRADE_DTO_H
#define UPGRADE_DTO_H


#include <cstdint>


struct UpgradeDTO {


    uint8_t handling_increase;
    uint8_t speed_increase;
    uint8_t acceleration_increase;
    uint16_t health_increase;


    explicit UpgradeDTO(uint8_t handling_increase = 0, uint8_t speed_increase = 0,
                        uint8_t acceleration_increase = 0, uint16_t health_increase = 0);
};


#endif
