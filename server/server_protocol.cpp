#include "common/constants.h"

#include "server_protocol.h"


ClientRequest ServerProtocol::convert_byte_to_request(const uint8_t byte) const {

    switch (byte) {

        case NO_REQUEST:
            return ClientRequest::NO_REQUEST;
        case MOVE_CAR:
            return ClientRequest::MOVE_CAR;
        default:
            return ClientRequest::UNKNOWN;
    }
}

CarMovement ServerProtocol::convert_byte_to_car_movement(const uint8_t byte) const {

    switch (byte) {

        case ACCELERATE:
            return CarMovement::ACCELERATE;
        case BRAKE:
            return CarMovement::BRAKE;
        case TURN_LEFT:
            return CarMovement::TURN_LEFT;
        case TURN_RIGHT:
            return CarMovement::TURN_RIGHT;
        default:
            return CarMovement::ACCELERATE;
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

CarMovement ServerProtocol::receive_car_movement() {

    const uint8_t byte = recv_uint8();

    return convert_byte_to_car_movement(byte);
}
