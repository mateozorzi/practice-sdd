#include "upgrade_dto.h"


UpgradeDTO::UpgradeDTO(const uint8_t handling_increase, const uint8_t speed_increase, const uint8_t acceleration_increase,
               const uint16_t health_increase):
            handling_increase(handling_increase),
            speed_increase(speed_increase),
            acceleration_increase(acceleration_increase),
            health_increase(health_increase) {}
