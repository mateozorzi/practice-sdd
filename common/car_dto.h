#ifndef CAR_DTO_H
#define CAR_DTO_H


#include <string>
#include <cstdint>

struct car_dto
{
    std::string model;
    uint8_t health;
    uint8_t handling;
    uint8_t speed;
    uint8_t acceleration;

    car_dto(const std::string& name, uint8_t health, uint8_t handling, uint8_t speed, uint8_t acceleration):
        model(name), health(health), handling(handling), speed(speed), acceleration(acceleration) {}
};

#endif  // CAR_DTO_H


