#include "server_protocol.h"

#include "../common/constants.h"


ClientRequest ServerProtocol::convert_byte_to_request(const uint8_t byte) const {

    switch (static_cast<ClientAction>(byte)) {

        case ClientAction::DISCONNECT:
            return ClientRequest::DISCONNECT;
        case ClientAction::SEND_USERNAME:
            return ClientRequest::SEND_USERNAME;
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

MovementDTO ServerProtocol::receive_movement() {

    const uint8_t direction_byte = recv_uint8();
    const uint8_t is_moving_byte = recv_uint8();

    Direction direction = static_cast<Direction>(direction_byte);
    bool is_moving = (is_moving_byte != 0);

    return MovementDTO(direction, is_moving);
}

void ServerProtocol::send_client_id(const uint32_t client_id) {

    add_uint32_to_buffer(client_id);

    send_buffer();
}

void ServerProtocol::send_world_snapshot(const WorldSnapshot& world_snapshot) {

    // add_uint8_to_buffer(world_snapshot.get_size());

    for (const auto& car_position : world_snapshot.get_car_positions())
        add_car_position_to_buffer(car_position);

    send_buffer();
}
