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


void ServerProtocol::send_car_position(const CarPositionDTO& car_position_dto) {

    add_uint32_to_buffer(car_position_dto.x_position);
    add_uint32_to_buffer(car_position_dto.y_position);
    add_uint32_to_buffer(car_position_dto.angle);
}
