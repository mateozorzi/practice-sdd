#include "player.h"
#include <iostream>

#include "../common/constants.h"
#include "../common/race_dto.h"
#include "../common/checkpoint_dto.h"
#include "../common/player_race_stats_dto.h"


Player::Player(uint32_t client_id, const std::string& username, b2World& world,
               const ServerConfig& config):
        client_id(client_id),
        username(username),
        car(std::make_unique<Car>(client_id, world, config)),
        health(config.app.server.world.physics.car.health),
        races_won(0),
        in_game(false),
        in_race(false), 
        has_finish_race(false),
        id_next_checkpoint(1),
        n_lap(1), next(), race_time(0) {}


uint32_t Player::get_client_id() const { return client_id; }

b2Body* Player::get_car_body() const {

    if (car)
        return car->get_body();

    return nullptr;
}

void Player::set_position_for_race(const std::vector<CheckpointDTO>& checkpoints) {

    if (car){
        std::cout << "[Player] next checkpoint id for player " << client_id << ": " << static_cast<int>(checkpoints[0].id) << std::endl;
        this->next = checkpoints[0];
        car->set_position_for_race(checkpoints);
    }
}

void Player::start_race(){
    in_race = true;
    has_finish_race = false;
    id_next_checkpoint = 1;
    n_lap = 1;
    race_time = 0;

}

bool Player::is_in_race() const {
    return in_race;
}

void Player::finish_race(){
    in_race = false;
    has_finish_race = true;
    id_next_checkpoint = 1;
    n_lap = 1;
    race_time = 0;
    
}

bool Player::has_finished_race()const{
    return has_finish_race;
}

bool Player::have_time_recorded() const {
    if (race_time > 0){
        return true;
    }
    return false;
}

void Player::record_time(uint32_t seconds){ 
    race_time = seconds;
}

uint32_t Player::get_recorded_time() const {
    return race_time;
}

PlayerRaceStatsDTO Player::get_race_stats() const {
    return PlayerRaceStatsDTO(client_id, username, 0, 0, race_time);
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

bool Player::reached_checkpoint(uint32_t x, uint32_t y, const CheckpointDTO& checkpoint){
    bool reached = false;

    if (static_cast<int>(checkpoint.id) != id_next_checkpoint){
        return reached;
    }

    //conteined in checkpoint?
    if ( (static_cast<int>(x) <= static_cast<int>(checkpoint.x) + (static_cast<int>(checkpoint.width/2)) && static_cast<int>(x) >= static_cast<int>(checkpoint.x) - (static_cast<int>(checkpoint.width/2))) &&
            (static_cast<int>(y) <= static_cast<int>(checkpoint.y) + (static_cast<int>(checkpoint.height/2)) && static_cast<int>(y) >= static_cast<int>(checkpoint.y) - (static_cast<int>(checkpoint.height/2))) ){
        //player reached checkpoint
        reached = true;
    }
    return reached;
}

void Player::update_next_checkpoint(const RaceDTO& race){
    if (static_cast<int>(id_next_checkpoint) >= static_cast<int>(race.size_checkpoints)) {
        // Último checkpoint de la vuelta
        //std::cout<<"Player id "<<client_id<<" reached finish line checkpoint "<< static_cast<int>(id_next_checkpoint)<<std::endl;
        if (n_lap+1 > race.laps) {
            finish_race();
            return;
        } else {
            //std::cout<<"Player id "<<client_id<<" completed lap "<< static_cast<int>(n_lap)<<std::endl;
            n_lap += 1;
            id_next_checkpoint = 1;  // Reset a primer checkpoint
        }
    } else if (static_cast<int>(id_next_checkpoint) < static_cast<int>(race.size_checkpoints)) {
        // Checkpoint normal
        //std::cout<<"Player id "<<client_id<<" reached checkpoint "<< static_cast<int>(id_next_checkpoint)<<std::endl;
        id_next_checkpoint += 1;
    }
    
    this->next = race.checkpoints[id_next_checkpoint-1];
    //std::cout<<"Player id "<<client_id<<" - Lap: "<< static_cast<int>(n_lap)<< 
    //", Next checkpoint: "<<static_cast<int>(this->next.id)<<" x: "<<static_cast<int>(this->next.x)<<" y: "<<static_cast<int>(this->next.y)<<std::endl;
    
}

void Player::update_car_position(const RaceDTO& race) {

    if (car){
        car->update_position();
        //update race conditions

        //check if player reached next checkpoint
        CarPositionDTO actual = car->get_position_dto();
        uint32_t x = actual.x_position;
        uint32_t y = actual.y_position;

        
        if (reached_checkpoint(x, y, this->next)){
            update_next_checkpoint(race);
            //std::cout<<"Player id "<<client_id<<" reached checkpoint "<< static_cast<int>(id_next_checkpoint)<< " next checkpoint id "<<static_cast<int>(this->next.id)<<std::endl;
            
            //std::cout<<"Position x: "<<static_cast<float>(x)<<" y: "<<static_cast<float>(y)<<std::endl;
        } else {
            return;
        }
    }
}

CarPositionDTO Player::get_car_position() const {

    if (car){
        CarPositionDTO actual = car->get_position_dto();
        actual.next = this->next;
        return actual;
    }
    return CarPositionDTO(client_id, 0, 0, 0);
}

void Player::set_car_collision(bool colliding) {

    if (car)
        car->set_collision(colliding);
}

bool Player::is_in_game() const { return in_game; }

void Player::set_in_game() { in_game = true; }


void Player::set_car_configuration(const car_dto& car_configuration, const CarClientConfig& car_config) {
    if (car){
        health = static_cast<uint16_t>(car_config.health);
        car->set_configuration(car_configuration, car_config);
    }
}

CarLayer Player::get_car_layer() const {

    if (car)
        return car->get_layer();
    
    return CarLayer::ON_GROUND;
}

void Player::set_car_layer(CarLayer layer) {

    if (car)
        car->set_layer(layer);
}
