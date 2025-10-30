#include "server_protocol.h"

#include "../common/constants.h"


ClientRequest ServerProtocol::convert_byte_to_request(const uint8_t byte) const {

    switch (byte) {

        case DISCONNECT:
            return ClientRequest::DISCONNECT;
        case MOVE_CAR:
            return ClientRequest::MOVE_CAR;
        default:
            return ClientRequest::UNKNOWN;
    }
}


void ServerProtocol::send_car_position(const CarPositionDTO& car_position_dto) {

    add_uint32_to_buffer(car_position_dto.x_position);
    add_uint32_to_buffer(car_position_dto.y_position);
    add_uint32_to_buffer(car_position_dto.angle);

    send_buffer();
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
