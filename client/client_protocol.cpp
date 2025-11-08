#include "client_protocol.h"
#include "../common/car_catalogue_dto.h"
#include "../common/car_dto.h"
#include "../common/lobby_dto.h"
#include <iostream>


ClientProtocol::ClientProtocol(const std::string& hostname, const std::string& port):
        CommonProtocol(client_socket), client_socket(hostname.c_str(), port.c_str()) {}

void ClientProtocol::send_username(const std::string& username) {
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_USERNAME));
    add_string_to_buffer(username);
    send_buffer();
}

void ClientProtocol::send_movement(const MovementDTO& movement) {
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_CAR_MOVEMENT));
    add_uint8_to_buffer(static_cast<uint8_t>(movement.direction));
    add_uint8_to_buffer(static_cast<uint8_t>(movement.isMoving ? 1 : 0));
    send_buffer();
}

void ClientProtocol::send_create_lobby(const LobbyDTO& lobby) {
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_CREATE_LOBBY));
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.lobby_id));
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.owner_id));
    add_string_to_buffer(lobby.lobby_name);
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.max_players));
    send_buffer();
}

void ClientProtocol::send_join_lobby(uint32_t lobby_id) {
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_JOIN_LOBBY));
    add_uint32_to_buffer(lobby_id);
    send_buffer();
}
void ClientProtocol::send_car_selection(const car_dto& car){
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_CAR_SELECTION));
    add_string_to_buffer(car.model);
    send_buffer();
}

CarPositionDTO ClientProtocol::receive_car_position() {

    const uint32_t client_id = recv_uint32();
    const uint32_t x_position = recv_uint32();
    const uint32_t y_position = recv_uint32();
    const uint32_t angle = recv_uint32();
    return CarPositionDTO(client_id, x_position, y_position, angle);
}

std::vector<CarPositionDTO> ClientProtocol::receive_car_positions() {
    const uint8_t size = recv_uint8();
    std::vector<CarPositionDTO> positions;
    for (uint8_t i = 0; i < size; ++i) {
        positions.push_back(receive_car_position());
    }
    return positions;
}

void ClientProtocol::ask_for_catalogue(){
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::GET_CAR_CATALOGUE));
    send_buffer();
}

CarCatalogueDTO ClientProtocol::receive_car_catalogue(){
    uint8_t size = recv_uint8();
    std::vector<car_dto> cars;
    for (uint8_t i = 0; i < size; ++i) {
        std::string model = recv_string();
        uint8_t health = recv_uint8();
        uint8_t handling = recv_uint8();
        uint8_t speed = recv_uint8();
        uint8_t acceleration = recv_uint8();
        cars.push_back(car_dto(model, health, handling, speed, acceleration));
    }
    return CarCatalogueDTO(std::move(cars));
}

uint32_t ClientProtocol::receive_id() { return recv_uint32(); }
