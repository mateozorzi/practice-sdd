#include "server_protocol.h"

#include <string>
#include <fstream>
#include <sstream>

#include "../common/constants.h"
#include <iostream>


void ServerProtocol::add_car_position_to_buffer(const CarPositionDTO& car_position_dto) {
    add_uint32_to_buffer(car_position_dto.client_id);
    add_uint32_to_buffer(car_position_dto.x_position);
    add_uint32_to_buffer(car_position_dto.y_position);
    add_uint32_to_buffer(car_position_dto.angle);
    add_checkpoint_to_buffer(car_position_dto.next);
    add_uint8_to_buffer(static_cast<uint8_t>(car_position_dto.layer));
}

void ServerProtocol::add_player_to_buffer(const PlayerDTO& player){

    add_string_to_buffer(player.username);
    add_uint32_to_buffer(player.id);
    add_string_to_buffer(player.car.model);
}

PlayerDTO ServerProtocol::receive_username() {

    const std::string& username = recv_string();

    return PlayerDTO(username);
}

LobbyDTO ServerProtocol::receive_lobby_info() {

    const uint8_t action_byte = recv_uint8();
    const uint32_t lobby_id = recv_uint32();
    const std::string lobby_name = recv_string();
    const uint8_t max_players = recv_uint8();
    const uint8_t active_players = recv_uint8();

    RaceCatalogueDTO selected_races = receive_races();


    LobbyAction action = static_cast<LobbyAction>(action_byte);
    std::cout<<"Received lobby info with action: "<<static_cast<int>(action_byte)<<std::endl;

    return LobbyDTO(action, lobby_id, lobby_name, max_players, active_players, selected_races);
}

MovementDTO ServerProtocol::receive_movement() {

    const uint8_t direction_byte = recv_uint8();
    const uint8_t is_moving_byte = recv_uint8();

    Direction direction = static_cast<Direction>(direction_byte);
    bool is_moving = (is_moving_byte != 0);

    return MovementDTO(direction, is_moving);
}


GameCommandDTO ServerProtocol::receive_game_command() {

    const uint8_t action_byte = recv_uint8();
    const ClientAction client_action = static_cast<ClientAction>(action_byte);

    switch (client_action) {

        case ClientAction::SEND_USERNAME:
            return GameCommandDTO(client_action, receive_username());
        case ClientAction::SEND_CAR_MOVEMENT:
            return GameCommandDTO(client_action, receive_movement());
        case ClientAction::SEND_CAR_SELECTION:
            return GameCommandDTO(client_action, receive_car_selection());
        case ClientAction::GET_CAR_CATALOGUE:
        case ClientAction::DISCONNECT:
            return GameCommandDTO(client_action);
        case ClientAction::GET_JSON_FILE:
            return GameCommandDTO(client_action, recv_string());
        default:
            return GameCommandDTO(ClientAction::DISCONNECT);
    }
}


void ServerProtocol::send_car_catalogue(const CarCatalogueDTO& car_catalogue_dto) {

    add_uint8_to_buffer(car_catalogue_dto.size);

    for (const auto& car: car_catalogue_dto.cars) {
        add_string_to_buffer(car.model);
        add_uint8_to_buffer(car.health);
        add_uint8_to_buffer(car.handling);
        add_uint8_to_buffer(car.speed);
        add_uint8_to_buffer(car.acceleration);
    }

    send_buffer();
}

car_dto ServerProtocol::receive_car_selection() {

    const std::string model = recv_string();

    return car_dto(model, 0, 0, 0, 0);
}

RaceDTO ServerProtocol::receive_race(){
    const std::string name = recv_string();
    const std::string city = recv_string();
    const uint8_t size_checkpoints = recv_uint8();
    std::vector<CheckpointDTO> checkpoints;
    for(uint8_t i =0; i < size_checkpoints; ++i){
        const uint8_t id = recv_uint8();
        const char orientation = static_cast<char>(recv_string()[0]);
        const uint16_t x = recv_uint16();
        const uint16_t y = recv_uint16();
        const uint8_t width = recv_uint8();
        const uint8_t height = recv_uint8();
        checkpoints.push_back(CheckpointDTO(id, orientation, x, y, width, height));
    }
    const uint8_t laps = recv_uint8();
    return RaceDTO(name, city, std::move(checkpoints), laps);
}

RaceCatalogueDTO ServerProtocol::receive_races(){
    uint8_t size = recv_uint8();
    std::vector<RaceDTO> races;
    for (uint8_t i = 0; i < size; ++i) {
        races.push_back(receive_race());
    }

    return RaceCatalogueDTO(std::move(races));
}

void ServerProtocol::send_client_id(const uint32_t client_id) {

    add_uint32_to_buffer(client_id);

    send_buffer();
}

void ServerProtocol::send_world_snapshot(const WorldSnapshotDTO& world_snapshot) {
    add_uint8_to_buffer(static_cast<uint8_t>(GameLoopAction::SEND_UPDATE_POSITIONS));
    add_uint8_to_buffer(world_snapshot.get_size());

    for (const auto& car_position: world_snapshot.get_car_positions()) {
        add_player_to_buffer(car_position.first);
        add_car_position_to_buffer(car_position.second);
    }

    send_buffer();
}

void ServerProtocol::send_race_update(const RaceDTO& race){    
    add_uint8_to_buffer(static_cast<uint8_t>(GameLoopAction::SEND_RACE));

    add_race_to_buffer(race);
    send_buffer();
}

void ServerProtocol::send_finish_race_timeout() {
    add_uint8_to_buffer(static_cast<uint8_t>(GameLoopAction::SEND_RACE_FINISHED));
    send_buffer();
}

void ServerProtocol::send_lobby(const LobbyDTO& lobby_dto) {
    add_uint8_to_buffer(static_cast<uint8_t>(lobby_dto.action));
    add_uint32_to_buffer(lobby_dto.lobby_id);
    add_string_to_buffer(lobby_dto.lobby_name);
    add_uint8_to_buffer(static_cast<uint8_t>(lobby_dto.max_players));
    add_uint8_to_buffer(static_cast<uint8_t>(lobby_dto.active_players));
}

void ServerProtocol::send_lobby_catalogue(const LobbyCatalogueDTO& lobby_catalogue) {

    add_uint8_to_buffer(lobby_catalogue.size);

    for (const auto& lobby: lobby_catalogue.lobbies) {
        send_lobby(lobby);
    }

    send_buffer();
}

void ServerProtocol::add_checkpoint_to_buffer(const CheckpointDTO& checkpoint){
    add_uint8_to_buffer(checkpoint.id);
    add_string_to_buffer(std::string(1, checkpoint.orientation));
    add_uint16_to_buffer(checkpoint.x);
    add_uint16_to_buffer(checkpoint.y);
    add_uint8_to_buffer(checkpoint.width);
    add_uint8_to_buffer(checkpoint.height);
}

void ServerProtocol::add_race_to_buffer(const RaceDTO& race){
    add_string_to_buffer(race.name);
    add_string_to_buffer(race.city);
    add_uint8_to_buffer(race.size_checkpoints);
    for (auto& c : race.checkpoints){
        add_checkpoint_to_buffer(c);
    }
    add_uint8_to_buffer(race.laps);
}

void ServerProtocol::send_races_catalogue(const RaceCatalogueDTO& races) {
    add_uint8_to_buffer(races.size);
    
    for (const auto& race: races.races) {
        add_race_to_buffer(race);
    }
    send_buffer();
    std::cout<<"Sent races catalogue of size: "<<static_cast<int>(races.size)<<std::endl;
}

void ServerProtocol::send_json_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        // Send empty JSON object if file doesn't exist
        add_string_to_buffer("{}");
        send_buffer();
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();
    add_string_to_buffer(json_content);
    send_buffer();
}
