#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <variant>

#include "../common/car_position_dto.h"
#include "../common/common_protocol.h"
#include "../common/constants.h"
#include "../common/movement_dto.h"
#include "../common/car_catalogue_dto.h"
#include "../common/lobby_dto.h"
#include "../common/lobbies_catalogue_dto.h"
#include "../common/world_snapshot_dto.h"
#include "../common/race_dto.h"
#include "../common/race_catalogue_dto.h"

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

    void ask_for_race_catalogue();
    void send_username(const std::string& username);
    void send_movement(const MovementDTO& movement);
    void send_create_lobby(const LobbyDTO& lobby);
    void ask_for_lobbies(const LobbyDTO& lobby);
    void send_action_lobby(const LobbyDTO& lobby);
    void send_car_selection(const car_dto& car);
    void change_username(const LobbyDTO& lobby);
    void send_exit(const LobbyDTO& lobby);
    void send_race(const RaceDTO& race);
    void send_races_selection(const std::vector<RaceDTO>& races);
    CarPositionDTO receive_car_position();
    std::vector<CarPositionDTO> receive_car_positions();
    void ask_for_catalogue();
    CarCatalogueDTO receive_car_catalogue();
    LobbyCatalogueDTO receive_lobby_catalogue();
    LobbyDTO receive_lobby_info();
    LobbyDTO receive_lobby_action();
    LobbyDTO receive_lobby();
    RaceDTO receive_race();
    RaceCatalogueDTO receive_race_catalogue();
    uint32_t receive_id();
    WorldSnapshotDTO receive_update_positions();
    PlayerDTO receive_player();
    nlohmann::json get_json(const std::string& file_path);
    std::variant<WorldSnapshotDTO, RaceDTO> receive_update();
};

#endif
