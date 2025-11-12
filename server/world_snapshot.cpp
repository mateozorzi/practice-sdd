#include "world_snapshot.h"


void WorldSnapshot::add_car_position(const CarPositionDTO& car_position) {

    car_positions.push_back(car_position);
}

const std::vector<CarPositionDTO>& WorldSnapshot::get_car_positions() const {

    return car_positions;
}

void WorldSnapshot::reserve(const size_t count) { car_positions.reserve(count); }


size_t WorldSnapshot::get_size() const { return car_positions.size(); }
