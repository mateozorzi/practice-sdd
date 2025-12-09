#include "car_dto.h"


CarDTO::CarDTO(const std::string& name, const uint16_t health, const uint8_t handling,
               const uint8_t speed, const uint8_t acceleration):
        model(name), health(health), handling(handling), speed(speed), acceleration(acceleration) {}
