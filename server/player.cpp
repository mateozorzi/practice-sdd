#include "player.h"
#include <iostream>


Player::Player(uint32_t client_id, const std::string& username, b2World& world):
        client_id(client_id),
        username(username),
        car(std::make_unique<Car>(client_id, world)),
        health(100),
        races_won(0),
        in_game(false) {}


uint32_t Player::get_client_id() const { return client_id; }

b2Body* Player::get_car_body() const {

    if (car)
        return car->get_body();

    return nullptr;
}

const std::string& Player::get_username() const { return username; }

const car_dto Player::get_car() const {
    car_dto empty_car;
    if (car) {
        return car->get_configuration();
    }
    return empty_car;
}


void Player::handle_car_movement(const MovementDTO& movement_dto) {

    if (car)
        car->handle_movement(movement_dto);
}

void Player::update_car_position() {

    if (car)
        car->update_position();
}

CarPositionDTO Player::get_car_position() const {

    if (car)
        return car->get_position_dto();

    return CarPositionDTO(client_id, 0, 0, 0);
}

void Player::set_car_collision(bool colliding) {

    if (car)
        car->set_collision(colliding);
}


bool Player::is_in_game() const { return in_game; }

void Player::set_in_game() { in_game = true; }


void Player::set_car_configuration(const car_dto& car_configuration) {
    if (car)
        car->set_configuration(car_configuration);
}
