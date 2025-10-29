#ifndef CAR_H
#define CAR_H


#include <box2d/box2d.h>

#include "common/car_position_dto.h"
#include "common/movement_dto.h"


class Car {

    b2Body* body;
    uint32_t x;
    uint32_t y;
    uint32_t angle;

    bool moving_up = false;
    bool moving_down = false;
    bool moving_left = false;
    bool moving_right = false;


public:
    Car(const Car&) = delete;
    Car& operator=(const Car&) = delete;

    explicit Car(b2World& world);

    void handle_movement(const MovementDTO& movement_dto);
    bool update_position();
    
    CarPositionDTO get_position_dto() const;
};

#endif
