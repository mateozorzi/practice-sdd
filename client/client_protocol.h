#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <string>
#include <vector>

#include "../common/car_position_dto.h"
#include "../common/common_protocol.h"
#include "../common/constants.h"
#include "../common/movement_dto.h"
#include "../common/car_catalogue_dto.h"
#include "../common/lobby_dto.h"
#include "../common/lobbies_catalogue_dto.h"
#include "../common/world_snapshot_dto.h"

enum class ServerResponse {

    SEND_INITIAL_BALANCE,
    GET_CURRENT_CAR,
    SEND_MARKET_INFO,
    SEND_CAR_BOUGHT,
    SEND_ERROR_MESSAGE,
    UNKNOWN
};

class ClientProtocol final: public CommonProtocol {

public:
    ClientProtocol(const ClientProtocol& other) = delete;
    ClientProtocol& operator=(const ClientProtocol& other) = delete;

    using CommonProtocol::CommonProtocol;

    void send_username(const std::string& username);
    void send_movement(const MovementDTO& movement);
    void send_create_lobby(const LobbyDTO& lobby);
    void ask_for_lobbies(const LobbyDTO& lobby);
    void send_action_lobby(const LobbyDTO& lobby);
    void send_car_selection(const car_dto& car);
    CarPositionDTO receive_car_position();
    std::vector<CarPositionDTO> receive_car_positions();
    void ask_for_catalogue();
    CarCatalogueDTO receive_car_catalogue();
    LobbyCatalogueDTO receive_lobby_catalogue();
    LobbyDTO receive_lobby_info();
    LobbyDTO receive_lobby_action();
    uint32_t receive_id();
    WorldSnapshotDTO receive_update_positions();
    PlayerDTO receive_player();
};

#endif
