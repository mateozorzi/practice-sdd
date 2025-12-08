#include "server_protocol.h"

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../common/client_event_dto.h"
#include "../common/constants.h"
#include "../common/game_update_dto.h"
#include "../common/leaderboard_dto.h"
#include "../common/movement_dto.h"
#include "../common/upgrade_dto.h"


void ServerProtocol::add_car_position_to_buffer(const CarPositionDTO& car_position_dto) {

    add_uint32_to_buffer(car_position_dto.client_id);
    add_uint32_to_buffer(car_position_dto.x_position);
    add_uint32_to_buffer(car_position_dto.y_position);
    add_uint32_to_buffer(car_position_dto.angle);
    add_checkpoint_to_buffer(car_position_dto.next);
    add_uint8_to_buffer(static_cast<uint8_t>(car_position_dto.layer));
    add_uint8_to_buffer(car_position_dto.current_lap);
    add_uint8_to_buffer(car_position_dto.race_lap);
}

void ServerProtocol::add_player_to_buffer(const PlayerDTO& player) {

    add_string_to_buffer(player.username);
    add_uint32_to_buffer(player.id);
    add_string_to_buffer(player.car.model);
    add_uint16_to_buffer(player.health);
    add_uint8_to_buffer(static_cast<uint8_t>(player.dead));
    add_uint16_to_buffer(player.max_health);
    add_uint8_to_buffer(static_cast<uint8_t>(player.finish_race));
}

void ServerProtocol::add_checkpoint_to_buffer(const CheckpointDTO& checkpoint) {

    add_uint8_to_buffer(checkpoint.id);
    add_string_to_buffer(std::string(1, checkpoint.orientation));
    add_uint16_to_buffer(checkpoint.x);
    add_uint16_to_buffer(checkpoint.y);
    add_uint8_to_buffer(checkpoint.width);
    add_uint8_to_buffer(checkpoint.height);
    add_uint8_to_buffer(static_cast<uint8_t>(checkpoint.is_finish_line));
}

void ServerProtocol::add_race_to_buffer(const RaceDTO& race) {

    add_string_to_buffer(race.name);
    add_string_to_buffer(race.city);
    add_uint8_to_buffer(race.size_checkpoints);

    for (const auto& checkpoint: race.checkpoints) add_checkpoint_to_buffer(checkpoint);

    add_uint8_to_buffer(race.laps);
}

void ServerProtocol::add_game_to_buffer(const LobbyDTO& lobby_dto) {

    add_uint8_to_buffer(static_cast<uint8_t>(lobby_dto.action));
    add_uint32_to_buffer(lobby_dto.lobby_id);
    add_string_to_buffer(lobby_dto.lobby_name);
    add_uint8_to_buffer(static_cast<uint8_t>(lobby_dto.max_players));
    add_uint8_to_buffer(static_cast<uint8_t>(lobby_dto.active_players));
}


PlayerDTO ServerProtocol::receive_username() {

    const std::string& username = recv_string();

    return PlayerDTO(username);
}

CarDTO ServerProtocol::receive_car_selection() {

    const std::string model = recv_string();

    return CarDTO(model, 0, 0, 0, 0);
}

RaceDTO ServerProtocol::receive_race() {

    const std::string name = recv_string();
    const std::string city = recv_string();
    const uint8_t size_checkpoints = recv_uint8();
    std::vector<CheckpointDTO> checkpoints;

    for (uint8_t i = 0; i < size_checkpoints; ++i) {

        const uint8_t id = recv_uint8();
        const char orientation = static_cast<char>(recv_string()[0]);
        const uint16_t x = recv_uint16();
        const uint16_t y = recv_uint16();
        const uint8_t width = recv_uint8();
        const uint8_t height = recv_uint8();
        const bool is_finish_line = static_cast<bool>(recv_uint8());
        checkpoints.push_back(CheckpointDTO(id, orientation, x, y, width, height, is_finish_line));
    }

    const uint8_t laps = recv_uint8();

    return RaceDTO(name, city, std::move(checkpoints), laps);
}

RaceCatalogueDTO ServerProtocol::receive_races() {

    uint8_t size = recv_uint8();
    std::vector<RaceDTO> races;

    for (uint8_t i = 0; i < size; ++i) races.push_back(receive_race());

    return RaceCatalogueDTO(std::move(races));
}

MovementDTO ServerProtocol::receive_movement() {

    const uint8_t direction_byte = recv_uint8();
    const uint8_t is_moving_byte = recv_uint8();

    Direction direction = static_cast<Direction>(direction_byte);
    bool is_moving = (is_moving_byte != 0);

    return MovementDTO(direction, is_moving);
}

BuyUpgradeDTO ServerProtocol::receive_upgrade() {

    const uint8_t upgrade = recv_uint8();
    const ButtonType upgrade_type = static_cast<ButtonType>(upgrade);

    return BuyUpgradeDTO{upgrade_type};
}


LobbyDTO ServerProtocol::receive_lobby_info() {

    const uint8_t action_byte = recv_uint8();
    const uint32_t lobby_id = recv_uint32();
    const std::string lobby_name = recv_string();
    const uint8_t max_players = recv_uint8();
    const uint8_t active_players = recv_uint8();

    RaceCatalogueDTO selected_races = receive_races();

    LobbyAction action = static_cast<LobbyAction>(action_byte);

    return LobbyDTO(action, lobby_id, lobby_name, max_players, active_players, selected_races);
}

GameCommandDTO ServerProtocol::receive_game_command() {

    const uint8_t action_byte = recv_uint8();
    const ClientAction client_action = static_cast<ClientAction>(action_byte);

    switch (client_action) {

        case ClientAction::DISCONNECT:
            return GameCommandDTO(client_action);
        case ClientAction::SEND_CAR_SELECTION:
            return GameCommandDTO(client_action, receive_car_selection());
        case ClientAction::SEND_UPGRADE:
            return GameCommandDTO(client_action, receive_upgrade());
        case ClientAction::SEND_CAR_MOVEMENT:
            return GameCommandDTO(client_action, receive_movement());
        case ClientAction::CHEAT_WIN_RACE:
            return GameCommandDTO(client_action);
        case ClientAction::CHEAT_WIN_GAME:
            return GameCommandDTO(client_action);
        case ClientAction::CHEAT_INFINITE_HEALTH:
            return GameCommandDTO(client_action);
        default:
            return GameCommandDTO(ClientAction::DISCONNECT);
    }
}


void ServerProtocol::send_world_snapshot(const GameUpdateDTO& snapshot) {

    const WorldSnapshotDTO& world_snapshot = std::get<WorldSnapshotDTO>(snapshot.data);
    add_uint8_to_buffer(world_snapshot.size);

    for (const auto& car_position: world_snapshot.get_car_positions()) {

        add_player_to_buffer(car_position.first);
        add_car_position_to_buffer(car_position.second);
    }

    add_uint8_to_buffer(world_snapshot.npc_size);

    for (const auto& npc: world_snapshot.get_npc_positions()) {

        add_uint32_to_buffer(npc.npc_id);
        add_uint32_to_buffer(npc.x_position);
        add_uint32_to_buffer(npc.y_position);
        add_uint32_to_buffer(npc.angle);
    }

    send_buffer();
}

void ServerProtocol::send_race_update(const GameUpdateDTO& snapshot) {

    const RaceDTO& race = std::get<RaceDTO>(snapshot.data);
    add_race_to_buffer(race);
    send_buffer();
}

void ServerProtocol::send_leaderboard_update(const GameUpdateDTO& snapshot) {


    const LeaderboardDTO& leaderboard = std::get<LeaderboardDTO>(snapshot.data);
    add_string_to_buffer(leaderboard.race_name);
    add_uint8_to_buffer(leaderboard.n_players);

    for (const auto& player_stats: leaderboard.players) {

        add_uint32_to_buffer(player_stats.client_id);
        add_string_to_buffer(player_stats.username);
        add_uint8_to_buffer(player_stats.position);
        add_uint8_to_buffer(player_stats.points);
        add_uint32_to_buffer(player_stats.total_time);
    }

    send_buffer();
}

void ServerProtocol::send_action_info(const GameUpdateDTO& action_info) {

    const uint8_t info = std::get<uint8_t>(action_info.data);
    add_uint8_to_buffer(info);
    send_buffer();
}

void ServerProtocol::send_race_time(const GameUpdateDTO& race_time) {

    const uint32_t elapsed_seconds = std::get<uint32_t>(race_time.data);
    add_uint32_to_buffer(elapsed_seconds);
    send_buffer();
}


void ServerProtocol::send_player_id(const uint32_t player_id) {

    add_uint32_to_buffer(player_id);

    send_buffer();
}

void ServerProtocol::send_active_games(const LobbyCatalogueDTO& game_catalogue) {

    add_uint8_to_buffer(game_catalogue.size);

    for (const auto& game: game_catalogue.lobbies) add_game_to_buffer(game);

    send_buffer();
}

void ServerProtocol::send_races_catalogue(const RaceCatalogueDTO& races) {

    add_uint8_to_buffer(races.size);

    for (const auto& race: races.races) add_race_to_buffer(race);

    send_buffer();
}

void ServerProtocol::send_lobby_response(const LobbyDTO& lobby_dto) {

    add_game_to_buffer(lobby_dto);

    send_buffer();
}

void ServerProtocol::send_car_catalogue(const CarCatalogueDTO& car_catalogue_dto) {

    add_uint8_to_buffer(car_catalogue_dto.size);

    for (const auto& car: car_catalogue_dto.cars) {
        add_string_to_buffer(car.model);
        add_uint16_to_buffer(car.health);
        add_uint8_to_buffer(car.handling);
        add_uint8_to_buffer(car.speed);
        add_uint8_to_buffer(car.acceleration);
    }

    send_buffer();
}

void ServerProtocol::send_game_update(const GameUpdateDTO& game_update) {

    add_uint8_to_buffer(static_cast<uint8_t>(game_update.gameloop_action));

    switch (game_update.gameloop_action) {

        case GameLoopAction::SEND_UPDATE_POSITIONS:
            send_world_snapshot(game_update);
            break;
        case GameLoopAction::SEND_RACE:
            send_race_update(game_update);
            break;
        case GameLoopAction::SEND_LEADERBOARD:
            send_leaderboard_update(game_update);
            break;
        case GameLoopAction::START_SHOP:
        case GameLoopAction::COUNTDOWN_TIME:
        case GameLoopAction::SEND_UPGRADES_REMAINING:
        case GameLoopAction::SEND_UPGRADE_SHOP_TIME_REMAINING:
            send_action_info(game_update);
            break;
        case GameLoopAction::RACE_TIME:
            send_race_time(game_update);
            break;
        default:
            break;
    }

    send_buffer();
}


void ServerProtocol::send_disconnection() {

    add_uint8_to_buffer(static_cast<uint8_t>(GameLoopAction::DISCONNECT));
    send_buffer();
}
