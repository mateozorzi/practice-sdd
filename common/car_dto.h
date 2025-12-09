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

    explicit CarDTO(const std::string& name = "", uint16_t health = 0, uint8_t handling = 0,
                    uint8_t speed = 0, uint8_t acceleration = 0);
};


#endif
