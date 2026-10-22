#include "player.h"

#include <algorithm>

#include "../common/checkpoint_dto.h"
#include "../common/client_event_dto.h"
#include "../common/constants.h"
#include "../common/player_dto.h"
#include "../common/player_race_stats_dto.h"
#include "../common/race_dto.h"
#include "../common/upgrade_dto.h"

#include "server_config.h"
#include "server_constants.h"


namespace {

// Consumes `elapsed_seconds` of an effect; returns true when it expires in this call
bool advance_effect(double& remaining, double elapsed_seconds) {

    if (remaining <= 0.0)
        return false;

    remaining -= elapsed_seconds;
    if (remaining > EFFECT_TIME_EPSILON)
        return false;

    remaining = 0.0;
    return true;
}

}  // namespace


Player::Player(uint32_t player_id, const std::string& username):
        player_id(player_id), username(username), car(nullptr), infinite_health(false) {}


void Player::create_car(const std::string& car_model, const ServerConfig& server_config,
                        b2World& box2d_world) {

    car = std::make_unique<Car>(player_id, box2d_world, car_model, server_config);

    base_health = car->get_max_health();
    health = base_health;
}


uint32_t Player::get_id() const { return player_id; }

const std::string& Player::get_username() const { return username; }

const CarDTO Player::get_car() const {

    CarDTO empty_car;

    if (car) {
        return car->get_configuration();
    }
    return empty_car;
}

b2Body* Player::get_car_body() const {

    if (car)
        return car->get_body();

    return nullptr;
}

CarPositionDTO Player::get_car_position(const RaceDTO& race) const {

    if (car) {
        CarPositionDTO actual = car->get_position_dto();
        actual.next = this->next;
        actual.current_lap = n_lap;
        actual.race_lap = race.laps;
        return actual;
    }
    return CarPositionDTO(player_id, 0, 0, 0);
}

CarLayer Player::get_car_layer() const {

    if (car)
        return car->get_layer();

    return CarLayer::ON_GROUND;
}

uint32_t Player::get_recorded_time() const { return race_time; }

PlayerDTO Player::get_info() const {

    return PlayerDTO(username, player_id, get_car(), health, dead, base_health, has_finish_race);
}

PlayerRaceStatsDTO Player::get_race_stats() const {

    return PlayerRaceStatsDTO(player_id, username, 0, 0, race_time);
}

uint16_t Player::get_health() const { return health; }

float Player::get_max_speed() const { return car ? car->get_max_speed() : 0.0f; }

float Player::get_acceleration() const { return car ? car->get_acceleration() : 0.0f; }


void Player::start_race() {

    in_race = true;
    has_finish_race = false;
    id_next_checkpoint = 1;
    n_lap = 1;
    race_time = 0;
    dead = false;
    health = base_health;
    infinite_health = false;
    clear_effects();
}

bool Player::is_in_race() const { return in_race; }

void Player::finish_race() {
    in_race = false;
    has_finish_race = true;
    infinite_health = true;
    id_next_checkpoint = 1;
    n_lap = 1;
    car->reset_car_velocity();
}


bool Player::is_next_checkpoint(const uint32_t checkpoint_id) const {

    return static_cast<uint32_t>(id_next_checkpoint) == checkpoint_id;
}

bool Player::has_finished_race() const { return has_finish_race; }

bool Player::have_time_recorded() const {
    if (race_time > 0) {
        return true;
    }
    return false;
}

void Player::record_time(uint32_t seconds, const GameConfig& game_config) {

    race_time = seconds;

    if (penalty) {

        int time_added = n_penalty * game_config.penalty_per_upgrade;
        race_time += static_cast<uint32_t>(time_added);
        penalty = false;
        n_penalty = 0;
    }
}

bool Player::is_dead() const { return health == 0; }


void Player::handle_car_movement(const MovementDTO& movement_dto) {

    if (car) {
        if (is_in_race())
            car->handle_movement(movement_dto);
    }
}

bool Player::update_next_checkpoint(const RaceDTO& race) {

    bool finished = false;

    bool finish_line = false;

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

        if (n_lap == race.laps && id_next_checkpoint == race.size_checkpoints)
            finish_line = true;
    }

    this->next = race.checkpoints[id_next_checkpoint - 1];
    this->next.is_finish_line = finish_line;

    return finished;
}

void Player::trigger_checkpoint_collision() {

    if (!is_in_race())
        return;

    checkpoint_triggered = true;
}

void Player::update_car_position(const RaceDTO& race, uint32_t elapsed_seconds,
                                 const GameConfig& game_config) {

    if (car && is_in_race()) {

        car->update_position();

        if (health == 0 && !dead) {

            finish_race();
            dead = true;
        }

        if (checkpoint_triggered) {

            checkpoint_triggered = false;

            bool finished = update_next_checkpoint(race);
            if (finished)
                record_time(elapsed_seconds, game_config);
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

    if (car && n_penalty < MAX_UPGRADES_PER_RACE) {

        n_penalty += 1;

        penalty = true;
        if (upgrade.upgrade == ButtonType::Health) {

            health += static_cast<uint16_t>(game_config.health_upgrade);
            base_health += static_cast<uint16_t>(game_config.health_upgrade);
            return;
        }
        car->set_upgrade(upgrade, game_config);
    }
}

void Player::activate_infinite_health() { infinite_health = true; }

bool Player::is_in_game() const { return in_game; }

void Player::apply_collision_damage(uint16_t damage) {

    if (infinite_health || has_effect(PowerUpType::Shield))
        return;
    if (health > damage) {
        health -= damage;
    } else {
        health = 0;
        clear_effects();
    }
}

uint8_t Player::get_upgrades_remaining() const {
    return static_cast<uint8_t>(MAX_UPGRADES_PER_RACE - n_penalty);
}

void Player::apply_power_up(PowerUpType type, const PowerUpConfig& power_up_config) {

    if (!car || is_dead())
        return;

    switch (type) {

        case PowerUpType::Nitro:
            nitro_remaining = power_up_config.nitro_duration;
            car->set_power_multiplier(power_up_config.nitro_multiplier);
            break;
        case PowerUpType::Shield:
            shield_remaining = power_up_config.shield_duration;
            break;
        case PowerUpType::Repair: {
            unsigned int repaired = health + power_up_config.repair_amount;
            health = static_cast<uint16_t>(
                    std::min(repaired, static_cast<unsigned int>(base_health)));
            break;
        }
    }
}

void Player::advance_effects(double elapsed_seconds) {

    if (advance_effect(nitro_remaining, elapsed_seconds))
        car->set_power_multiplier(1.0f);

    advance_effect(shield_remaining, elapsed_seconds);
}

bool Player::has_effect(PowerUpType type) const { return get_effect_remaining(type) > 0.0; }

double Player::get_effect_remaining(PowerUpType type) const {

    switch (type) {
        case PowerUpType::Nitro:
            return nitro_remaining;
        case PowerUpType::Shield:
            return shield_remaining;
        case PowerUpType::Repair:
            break;
    }
    return 0.0;
}

void Player::clear_effects() {

    nitro_remaining = 0.0;
    shield_remaining = 0.0;

    if (car)
        car->set_power_multiplier(1.0f);
}
