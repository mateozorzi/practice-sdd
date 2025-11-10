#ifndef SERVER_PROTOCOL_h
#define SERVER_PROTOCOL_h

#include "../common/car_dto.h"
#include "../common/car_position_dto.h"
#include "../common/common_protocol.h"
#include "../common/car_catalogue_dto.h"
#include "../common/movement_dto.h"
#include "../common/player_dto.h"

#include "client_request.h"
#include "world_snapshot.h"


class ServerProtocol final: public CommonProtocol {


    ClientRequest convert_byte_to_request(uint8_t byte) const;
    void add_car_position_to_buffer(const CarPositionDTO& car_position_dto);


public:
    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;

    using CommonProtocol::CommonProtocol;

    ClientRequest receive_client_request();

    PlayerDTO receive_username();
    car_dto receive_car_selection();
    MovementDTO receive_movement();

    void send_client_id(uint32_t client_id);
    void send_car_catalogue(const CarCatalogueDTO& catalogue);
    void send_world_snapshot(const WorldSnapshot& world_snapshot);
};


#endif
