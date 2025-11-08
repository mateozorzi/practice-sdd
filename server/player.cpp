#include "player.h"


Player::Player(uint32_t client_id, const std::string& username, 
               b2World& world, const CarConfig& config)
    : client_id(client_id),
      username(username),
      car(std::make_unique<Car>(client_id, world, config)),
      health(100),
      races_won(0),
      in_game(false) {}


uint32_t Player::get_client_id() const {

    return client_id;
}

b2Body* Player::get_car_body() const {

    if (car)
        return car->get_body();

    return nullptr;
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


bool Player::is_in_game() const {

    return in_game;
}

void Player::set_in_game() {

    in_game = true;
}
