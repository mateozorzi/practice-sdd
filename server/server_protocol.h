#ifndef SERVER_PROTOCOL_h
#define SERVER_PROTOCOL_h


#include <string>

#include "../common/car_catalogue_dto.h"
#include "../common/client_event_dto.h"
#include "../common/common_protocol.h"
#include "../common/game_command_dto.h"
#include "../common/game_update_dto.h"
#include "../common/leaderboard_dto.h"
#include "../common/lobbies_catalogue_dto.h"
#include "../common/movement_dto.h"
#include "../common/player_dto.h"
#include "../common/race_catalogue_dto.h"
#include "../common/upgrade_dto.h"
#include "../common/world_snapshot_dto.h"


class ServerProtocol final: public CommonProtocol {

    void add_car_position_to_buffer(const CarPositionDTO& car_position_dto);
    void add_player_to_buffer(const PlayerDTO& player);
    PlayerDTO receive_username();
    CarDTO receive_car_selection();
    RaceDTO receive_race();
    RaceCatalogueDTO receive_races();
    MovementDTO receive_movement();
    BuyUpgradeDTO receive_upgrade();
    void add_race_to_buffer(const RaceDTO& race);
    void add_checkpoint_to_buffer(const CheckpointDTO& checkpoint);
    void send_lobby(const LobbyDTO& lobby_dto);


public:
    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;

    using CommonProtocol::CommonProtocol;

    LobbyDTO receive_lobby_info();
    GameCommandDTO receive_game_command();

    void send_client_id(uint32_t client_id);
    void send_car_catalogue(const CarCatalogueDTO& catalogue);
    void send_world_snapshot(const GameUpdateDTO& snapshot);
    void send_race_update(const GameUpdateDTO& snapshot);
    void send_leaderboard_update(const GameUpdateDTO& snapshot);
    void send_finish_race_timeout();
    void send_lobby_catalogue(const LobbyCatalogueDTO& lobby_catalogue);
    void send_races_catalogue(const RaceCatalogueDTO& races);
    void send_json_file(const std::string& file_path);
    void send_game_action(const GameUpdateDTO& game_action);
};


#endif
