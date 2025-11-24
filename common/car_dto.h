#ifndef CAR_DTO_H
#define CAR_DTO_H


#include <cstdint>
#include <string>

struct CarDTO {
    std::string model;
    uint16_t health;
    uint8_t handling;
    uint8_t speed;
    uint8_t acceleration;

    CarDTO(const std::string& name, uint16_t health, uint8_t handling, uint8_t speed,
           uint8_t acceleration):
            model(name),
            health(health),
            handling(handling),
            speed(speed),
            acceleration(acceleration) {}

    explicit CarDTO(const std::string& name):
            model(name), health(0), handling(0), speed(0), acceleration(0) {}

    CarDTO(): model(""), health(0), handling(0), speed(0), acceleration(0) {}
};

#endif  // CAR_DTO_H
