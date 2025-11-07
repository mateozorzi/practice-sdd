#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <string>
#include <vector>

#include "../common/car_position_dto.h"
#include "../common/common_protocol.h"
#include "../common/constants.h"
#include "../common/movement_dto.h"
#include "../common/car_catalogue_dto.h"

enum class ServerResponse {

    SEND_INITIAL_BALANCE,
    GET_CURRENT_CAR,
    SEND_MARKET_INFO,
    SEND_CAR_BOUGHT,
    SEND_ERROR_MESSAGE,
    UNKNOWN
};

class ClientProtocol final: public CommonProtocol {
    Socket client_socket;

public:
    ClientProtocol(const ClientProtocol& other) = delete;
    ClientProtocol& operator=(const ClientProtocol& other) = delete;

    explicit ClientProtocol(const std::string& hostname, const std::string& port);

    void send_username(const std::string& username);
    void send_movement(const MovementDTO& movement);
    CarPositionDTO receive_car_position();
    std::vector<CarPositionDTO> receive_car_positions();
    void ask_for_catalogue();
    CarCatalogueDTO receive_car_catalogue();
    uint32_t receive_id();
};

#endif
