#ifndef CAR_DTO_H
#define CAR_DTO_H


#include <cstdint>
#include <string>

struct car_dto {
    std::string model;
    uint8_t health;
    uint8_t handling;
    uint8_t speed;
    uint8_t acceleration;

    car_dto(const std::string& name, uint8_t health, uint8_t handling, uint8_t speed,
            uint8_t acceleration):
            model(name),
            health(health),
            handling(handling),
            speed(speed),
            acceleration(acceleration) {}

    explicit car_dto(const std::string& name):
            model(name), health(0), handling(0), speed(0), acceleration(0) {}

    car_dto(): model(""), health(0), handling(0), speed(0), acceleration(0) {}
};

#endif  // CAR_DTO_H
