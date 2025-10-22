#include "client_protocol.h"

ClientProtocol::ClientProtocol(const std::string& hostname, const std::string& port):
        CommonProtocol(client_socket), client_socket(hostname.c_str(), port.c_str()) {}

void ClientProtocol::send_username() {

    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_USERNAME));
    add_string_to_buffer("Juan");

    send_buffer();
}

void ClientProtocol::send_car_movement(const CarMovement movement) {
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_CAR_MOVEMENT));
    add_uint8_to_buffer(static_cast<uint8_t>(movement));
    send_buffer();
}
