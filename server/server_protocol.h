#ifndef SERVER_PROTOCOL_h
#define SERVER_PROTOCOL_h


#include "common/car_position_dto.h"
#include "common/movement_dto.h"
#include "common/common_protocol.h"

#include "client_request.h"


class ServerProtocol final: public CommonProtocol {


    ClientRequest convert_byte_to_request(uint8_t byte) const;


public:
    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;

    using CommonProtocol::CommonProtocol;

    void send_car_position(const CarPositionDTO& car_position_dto);

    ClientRequest receive_client_request();

    MovementDTO receive_movement();
};


#endif
