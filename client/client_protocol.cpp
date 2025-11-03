#include "client_protocol.h"

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

CarPositionDTO ClientProtocol::receive_car_position() {

    const uint32_t client_id = recv_uint32();
    const uint32_t x_position = recv_uint32();
    const uint32_t y_position = recv_uint32();
    const uint32_t angle = recv_uint32();
    return CarPositionDTO(client_id, x_position, y_position, angle);
}
