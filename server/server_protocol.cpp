#include "server_protocol.h"

#include "../common/constants.h"


ClientRequest ServerProtocol::convert_byte_to_request(const uint8_t byte) const {

    switch (static_cast<ClientAction>(byte)) {

        case ClientAction::DISCONNECT:
            return ClientRequest::DISCONNECT;
        case ClientAction::SEND_USERNAME:
            return ClientRequest::SEND_USERNAME;
        case ClientAction::GET_CAR_CATALOGUE:
            return ClientRequest::GET_CAR_CATALOGUE;
        case ClientAction::SEND_CAR_SELECTION:
            return ClientRequest::SEND_CAR_SELECTION;
        case ClientAction::SEND_CAR_MOVEMENT:
            return ClientRequest::MOVE_CAR;
        default:
            return ClientRequest::UNKNOWN;
    }
}


void ServerProtocol::add_car_position_to_buffer(const CarPositionDTO& car_position_dto) {

    add_uint32_to_buffer(car_position_dto.client_id);
    add_uint32_to_buffer(car_position_dto.x_position);
    add_uint32_to_buffer(car_position_dto.y_position);
    add_uint32_to_buffer(car_position_dto.angle);
}


ClientRequest ServerProtocol::receive_client_request() {

    const uint8_t byte = recv_uint8();

    return convert_byte_to_request(byte);
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

    LobbyAction action = static_cast<LobbyAction>(action_byte);

    return LobbyDTO(action, lobby_id, lobby_name, max_players);
}

MovementDTO ServerProtocol::receive_movement() {

    const uint8_t direction_byte = recv_uint8();
    const uint8_t is_moving_byte = recv_uint8();

    Direction direction = static_cast<Direction>(direction_byte);
    bool is_moving = (is_moving_byte != 0);

    return MovementDTO(direction, is_moving);
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

void ServerProtocol::send_client_id(const uint32_t client_id) {

    add_uint32_to_buffer(client_id);

    send_buffer();
}

void ServerProtocol::send_world_snapshot(const WorldSnapshot& world_snapshot) {

    add_uint8_to_buffer(world_snapshot.get_size());

    for (const auto& car_position : world_snapshot.get_car_positions())
        add_car_position_to_buffer(car_position);

    send_buffer();
}

void ServerProtocol::send_lobby_catalogue(const LobbyCatalogueDTO& lobby_catalogue) {

    add_uint8_to_buffer(lobby_catalogue.size);

    for (const auto& lobby : lobby_catalogue.lobbies)
        add_uint32_to_buffer(lobby.lobby_id);
    
    send_buffer();
}
