#include "player.h"

#include <iostream>

#include "../common/checkpoint_dto.h"
#include "../common/client_event_dto.h"
#include "../common/constants.h"
#include "../common/player_dto.h"
#include "../common/player_race_stats_dto.h"
#include "../common/race_dto.h"
#include "../common/upgrade_dto.h"

#include "server_config.h"


Player::Player(uint32_t client_id, const std::string& username):
        client_id(client_id), username(username), car(nullptr) {}


void Player::create_car(const CarDTO& car_dto, const CarConfig& car_config, b2World& box2d_world) {

    car = std::make_unique<Car>(client_id, box2d_world, car_dto.model, car_config);

    base_health = static_cast<uint16_t>(car_config.health);
    health = base_health;

    std::cout << "[" << PLAYER << "] Created car for player with ID: " << client_id
              << " | Car model: " << car_dto.model << " | HP: " << health << std::endl;
}


uint32_t Player::get_client_id() const { return client_id; }

const std::string& Player::get_username() const { return username; }

const CarDTO Player::get_car() const {

    CarDTO empty_car;

    if (car)
        return car->get_configuration();

    return empty_car;
}

b2Body* Player::get_car_body() const {

    if (car)
        return car->get_body();

    return nullptr;
}

CarPositionDTO Player::get_car_position() const {

    if (car) {
        CarPositionDTO actual = car->get_position_dto();
        actual.next = this->next;
        return actual;
    }
    return CarPositionDTO(client_id, 0, 0, 0);
}

CarLayer Player::get_car_layer() const {

    if (car)
        return car->get_layer();

    return CarLayer::ON_GROUND;
}

uint32_t Player::get_recorded_time() const { return race_time; }

PlayerDTO Player::get_info() const {

    return PlayerDTO(username, client_id, get_car(), health, dead);
}

PlayerRaceStatsDTO Player::get_race_stats() const {
    std::cout << "[" << PLAYER << "] Getting  race stats for player with ID: " << client_id
              << std::endl;
    std::cout << "Race time: " << race_time << std::endl;
    return PlayerRaceStatsDTO(client_id, username, 0, 0, race_time);
}

uint16_t Player::get_health() const { return health; }


void Player::start_race() {
    in_race = true;
    has_finish_race = false;
    id_next_checkpoint = 1;
    n_lap = 1;
    race_time = 0;
    dead = false;
    health = base_health;
}

bool Player::is_in_race() const { return in_race; }

void Player::finish_race() {
    in_race = false;
    has_finish_race = true;
    id_next_checkpoint = 1;
    n_lap = 1;
    car->reset_car_velocity();
}

bool Player::has_finished_race() const { return has_finish_race; }

bool Player::have_time_recorded() const {
    if (race_time > 0) {
        return true;
    }
    return false;
}

void Player::record_time(uint32_t seconds) {
    race_time = seconds;
    std::cout << "[" << PLAYER << "] Player " << client_id << " recorded time: " << race_time
              << " seconds." << std::endl;
    if (penalty) {
        race_time += static_cast<uint32_t>(10);
        std::cout << "[" << PLAYER << "] Penalty applied to player " << client_id
                  << ". New recorded time: " << race_time << " seconds." << std::endl;
        penalty = false;
    }
}

bool Player::is_dead() const { return health == 0; }


void Player::handle_car_movement(const MovementDTO& movement_dto) {

    if (car) {
        if (is_in_race()) {
            car->handle_movement(movement_dto);
        }
    }
}

bool Player::reached_checkpoint(uint32_t x, uint32_t y, const CheckpointDTO& checkpoint) {
    bool reached = false;

    if (static_cast<int>(checkpoint.id) != id_next_checkpoint) {
        return reached;
    }

    // conteined in checkpoint?
    if ((static_cast<int>(x) <=
                 static_cast<int>(checkpoint.x) + (static_cast<int>(checkpoint.width / 2)) &&
         static_cast<int>(x) >=
                 static_cast<int>(checkpoint.x) - (static_cast<int>(checkpoint.width / 2))) &&
        (static_cast<int>(y) <=
                 static_cast<int>(checkpoint.y) + (static_cast<int>(checkpoint.height / 2)) &&
         static_cast<int>(y) >=
                 static_cast<int>(checkpoint.y) - (static_cast<int>(checkpoint.height / 2)))) {
        // player reached checkpoint
        reached = true;
    }
    return reached;
}

bool Player::update_next_checkpoint(const RaceDTO& race) {
    bool finished = false;
    if (static_cast<int>(id_next_checkpoint) >= static_cast<int>(race.size_checkpoints)) {
        if (n_lap + 1 > race.laps) {
            finish_race();
            finished = true;
            return finished;
        } else {
            n_lap += 1;
            id_next_checkpoint = 1;
        }
    } else if (static_cast<int>(id_next_checkpoint) < static_cast<int>(race.size_checkpoints)) {
        id_next_checkpoint += 1;
    }

    this->next = race.checkpoints[id_next_checkpoint - 1];
    return finished;
}

void Player::update_car_position(const RaceDTO& race, uint32_t elapsed_seconds) {

    if (car && is_in_race()) {
        car->update_position();
        // update race conditions

        if (health == 0 && !dead) {
            finish_race();
            dead = true;
        }

        // check if player reached next checkpoint
        CarPositionDTO actual = car->get_position_dto();
        uint32_t x = actual.x_position;
        uint32_t y = actual.y_position;


        if (reached_checkpoint(x, y, this->next)) {
            bool finished = update_next_checkpoint(race);
            if (finished) {
                record_time(elapsed_seconds);
            }
        } else {
            return;
        }
    }
}

void Player::set_car_collision(bool colliding) {

    if (car)
        car->set_collision(colliding);
}

void Player::set_position_for_race(const std::vector<CheckpointDTO>& checkpoints) {

    if (car) {

        this->next = checkpoints[0];
        car->set_position_for_race(checkpoints);
    }
}

void Player::set_car_layer(CarLayer layer) {

    if (car)
        car->set_layer(layer);
}

void Player::set_in_game() { in_game = true; }

void Player::set_upgrade(const BuyUpgradeDTO& upgrade, const UpgradeConfig& game_config) {

    if (car) {
        penalty = true;
        if (upgrade.upgrade == ButtonType::Health) {
            std::cout << "Actual health: " << health << std::endl;
            health += static_cast<uint16_t>(game_config.health_upgrade);
            std::cout << "Upgraded health: " << health << std::endl;
            return;
        }
        car->set_upgrade(upgrade, game_config);
    }
}


bool Player::is_in_game() const { return in_game; }

void Player::apply_collision_damage(uint16_t damage) {

    if (health > damage)
        health -= damage;
    else
        health = 0;
}
