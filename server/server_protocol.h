#ifndef SERVER_PROTOCOL_h
#define SERVER_PROTOCOL_h


#include "../common/car_catalogue_dto.h"
#include "../common/common_protocol.h"
#include "../common/game_command_dto.h"
#include "../common/lobbies_catalogue_dto.h"
#include "../common/movement_dto.h"
#include "../common/player_dto.h"

#include "world_snapshot.h"


class ServerProtocol final: public CommonProtocol {

    void add_car_position_to_buffer(const CarPositionDTO& car_position_dto);
    PlayerDTO receive_username();
    car_dto receive_car_selection();
    MovementDTO receive_movement();


public:
    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;

    using CommonProtocol::CommonProtocol;

    LobbyDTO receive_lobby_info();
    GameCommandDTO receive_game_command();

    void send_client_id(uint32_t client_id);
    void send_car_catalogue(const CarCatalogueDTO& catalogue);
    void send_world_snapshot(const WorldSnapshot& world_snapshot);
    void send_lobby_catalogue(const LobbyCatalogueDTO& lobby_catalogue);
};


#endif
