#include "client_protocol.h"
#include "../common/car_catalogue_dto.h"
#include "../common/car_dto.h"
#include "../common/lobby_dto.h"
#include "../common/world_snapshot_dto.h"
#include "../common/race_dto.h"
#include "../common/race_catalogue_dto.h"
#include "../common/game_update_dto.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <variant>

void ClientProtocol::ask_for_race_catalogue(){
    std::cout<<"Requesting race catalogue from server..."<<std::endl;
    add_uint8_to_buffer(static_cast<uint8_t>(LobbyAction::LIST_RACES));
    add_uint32_to_buffer(0);
    add_string_to_buffer("");
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0); //race = 0
    send_buffer();
}

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

void ClientProtocol::send_create_lobby(const LobbyDTO& lobby) {
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.action));
    add_uint32_to_buffer(static_cast<uint32_t>(lobby.lobby_id));
    add_string_to_buffer(lobby.lobby_name);
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.max_players));
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0); //race = 0
    send_buffer();
}

void ClientProtocol::ask_for_lobbies(const LobbyDTO& lobby) {
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.action));
    add_uint32_to_buffer(0);
    add_string_to_buffer("");
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0); //race = 0
    send_buffer();
}

void ClientProtocol::change_username(const LobbyDTO& lobby) {
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.action));
    add_uint32_to_buffer(static_cast<uint32_t>(lobby.lobby_id));
    add_string_to_buffer(lobby.lobby_name);
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.max_players));
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0); //race = 0
    send_buffer();
}

void ClientProtocol::send_action_lobby(const LobbyDTO& lobby) {
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.action));
    add_uint32_to_buffer(static_cast<uint32_t>(lobby.lobby_id));
    add_string_to_buffer("");
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0); //race = 0
    send_buffer();
}

void ClientProtocol::send_exit(const LobbyDTO& lobby) {
    add_uint8_to_buffer(static_cast<uint8_t>(lobby.action));
    add_uint32_to_buffer(0);
    add_string_to_buffer("");
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0); //race = 0
    send_buffer();
}

void ClientProtocol::send_car_selection(const car_dto& car){
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::SEND_CAR_SELECTION));
    add_string_to_buffer(car.model);
    send_buffer();
}

void ClientProtocol::send_race(const RaceDTO& race){
    add_string_to_buffer(race.name);
    add_string_to_buffer(race.city);
    add_uint8_to_buffer(race.size_checkpoints);
    for (const auto& c : race.checkpoints){
        add_uint8_to_buffer(c.id);
        add_string_to_buffer(std::string(1, c.orientation));
        add_uint16_to_buffer(c.x);
        add_uint16_to_buffer(c.y);
        add_uint8_to_buffer(c.width);
        add_uint8_to_buffer(c.height);
    }
    add_uint8_to_buffer(race.laps);
}

void ClientProtocol::send_races_selection(const std::vector<RaceDTO>& races){
    add_uint8_to_buffer(static_cast<uint8_t>(LobbyAction::SEND_RACE_SELECTION));
    add_uint32_to_buffer(0);
    add_string_to_buffer("");
    add_uint8_to_buffer(0);
    add_uint8_to_buffer(0);
    uint8_t size = static_cast<uint8_t>(races.size());
    add_uint8_to_buffer(size);
    for (const auto& race : races) {
        send_race(race);
    }

    send_buffer();

}

CarPositionDTO ClientProtocol::receive_car_position() {

    const uint32_t client_id = recv_uint32();
    const uint32_t x_position = recv_uint32();
    const uint32_t y_position = recv_uint32();
    const uint32_t angle = recv_uint32();
    const CheckpointDTO next = receive_checkpoint();
    const uint8_t layer = recv_uint8();
    return CarPositionDTO(client_id, x_position, y_position, angle, next, static_cast<CarLayer>(layer));
}

std::vector<CarPositionDTO> ClientProtocol::receive_car_positions() {
    const uint8_t size = recv_uint8();
    std::vector<CarPositionDTO> positions;
    for (uint8_t i = 0; i < size; ++i) {
        positions.push_back(receive_car_position());
    }
    return positions;
}

PlayerDTO ClientProtocol::receive_player(){
    const std::string& username = recv_string();
    const uint32_t id = recv_uint32();
    const std::string car_model = recv_string();
    car_dto car(car_model);
    return PlayerDTO(username, id, car);
}

WorldSnapshotDTO ClientProtocol::receive_update_positions() {
    const uint8_t size = recv_uint8();
    WorldSnapshotDTO world_snapshot;
    for (uint8_t i = 0; i < size; ++i) {
        PlayerDTO player = receive_player();
        CarPositionDTO car_position = receive_car_position();
        world_snapshot.add_car_position(player, car_position);
    }
    return world_snapshot;
}

LeaderboardDTO ClientProtocol::receive_leaderboard(){
    std::string race_name = recv_string();
    uint8_t n_players = recv_uint8();
    std::vector<PlayerRaceStatsDTO> players;
    for (uint8_t i = 0; i < n_players; ++i) {
        uint32_t client_id = recv_uint32();
        std::string username = recv_string();
        uint8_t position = recv_uint8();
        uint8_t points = recv_uint8();
        uint32_t total_time_ms = recv_uint32();
        players.emplace_back(client_id, username, position, points, total_time_ms);
    }
    return LeaderboardDTO(race_name, players);

}

GameUpdateDTO ClientProtocol::receive_update() {
    std::variant<std::monostate, WorldSnapshotDTO, RaceDTO, LeaderboardDTO> data;
    const uint8_t update_type = recv_uint8();
    if (update_type == static_cast<uint8_t>(GameLoopAction::SEND_UPDATE_POSITIONS)) {
        data = receive_update_positions();
    } else if (update_type == static_cast<uint8_t>(GameLoopAction::SEND_RACE)) {
        data = receive_race();
    } else if (update_type == static_cast<uint8_t>(GameLoopAction::SEND_LEADERBOARD)) {
        data = receive_leaderboard();
    } else { //update_type == SEND_FINISH_RACE
        data = std::monostate{};
    }

    GameUpdateDTO update(static_cast<GameLoopAction>(update_type), data);
    return update;
}


void ClientProtocol::ask_for_catalogue(){
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::GET_CAR_CATALOGUE));
    send_buffer();
}

CarCatalogueDTO ClientProtocol::receive_car_catalogue(){
    uint8_t size = recv_uint8();
    std::vector<car_dto> cars;
    for (uint8_t i = 0; i < size; ++i) {
        std::string model = recv_string();
        uint16_t health = recv_uint16();
        uint8_t handling = recv_uint8();
        uint8_t speed = recv_uint8();
        uint8_t acceleration = recv_uint8();
        cars.push_back(car_dto(model, health, handling, speed, acceleration));
    }
    return CarCatalogueDTO(std::move(cars));
}

LobbyDTO ClientProtocol::receive_lobby(){
    const uint8_t action_byte = recv_uint8();
    std::cout<<"Received lobby action byte: "<<static_cast<int>(action_byte)<<std::endl;
    const uint32_t lobby_id = recv_uint32();
    std::cout<<"Received lobby ID: "<<static_cast<int>(lobby_id)<<std::endl;
    const std::string lobby_name = recv_string();
    std::cout<<"Received lobby name: "<<lobby_name<<std::endl;
    const uint8_t max_players = recv_uint8();
    std::cout<<"Received lobby max players: "<<static_cast<int>(max_players)<<std::endl;
    const uint8_t active_players = recv_uint8();
    std::cout<<"Received lobby active players: "<<static_cast<int>(active_players)<<std::endl;

    LobbyAction action = static_cast<LobbyAction>(action_byte);

    return LobbyDTO(action, lobby_id, lobby_name, max_players, active_players);
}

LobbyCatalogueDTO ClientProtocol::receive_lobby_catalogue(){
    uint8_t size = recv_uint8();
    std::cout<<"Received lobby catalogue of size: "<<static_cast<int>(size)<<std::endl;
    std::vector<LobbyDTO> lobbies;
    for (uint8_t i = 0; i < size; ++i) {
        LobbyDTO lobby = receive_lobby();
        lobbies.push_back(lobby);
        std::cout<<"Received lobby: "<<lobby.lobby_name<<std::endl;
    }
    return LobbyCatalogueDTO(std::move(lobbies));
}

CheckpointDTO ClientProtocol::receive_checkpoint(){
    const uint8_t id = recv_uint8();
    const char orientation = static_cast<char>(recv_string()[0]);
    const uint16_t x = recv_uint16();
    const uint16_t y = recv_uint16();
    const uint8_t width = recv_uint8();
    const uint8_t height = recv_uint8();
    return CheckpointDTO(id, orientation, x, y, width, height);
}

RaceDTO ClientProtocol::receive_race(){
    const std::string name = recv_string();
    const std::string city = recv_string();
    const uint8_t size_checkpoints = recv_uint8();
    std::vector<CheckpointDTO> checkpoints;
    for(uint8_t i =0; i < size_checkpoints; ++i){
        CheckpointDTO c = receive_checkpoint();
        checkpoints.push_back(c);
    }
    const uint8_t laps = recv_uint8();
    return RaceDTO(name, city, std::move(checkpoints), laps);
}

RaceCatalogueDTO ClientProtocol::receive_race_catalogue(){
    uint8_t size = recv_uint8();
    std::vector<RaceDTO> races;
    for (uint8_t i = 0; i < size; ++i) {
        races.push_back(receive_race());
    }
    std::cout<<"Received race catalogue of size: "<<static_cast<int>(size)<<std::endl;

    return RaceCatalogueDTO(std::move(races));
}

LobbyDTO ClientProtocol::receive_lobby_info(){
    const uint8_t action_byte = recv_uint8();
    const uint32_t lobby_id = recv_uint32();
    //const std::string lobby_name = recv_string();
    //const uint8_t max_players = recv_uint8();

    LobbyAction action = static_cast<LobbyAction>(action_byte);

    return LobbyDTO(action, lobby_id);
}

LobbyDTO ClientProtocol::receive_lobby_action(){
    const uint8_t action_byte = recv_uint8();

    LobbyAction action = static_cast<LobbyAction>(action_byte);

    return LobbyDTO(action);
}

uint32_t ClientProtocol::receive_id() { return recv_uint32(); }

nlohmann::json ClientProtocol::get_json(const std::string& file_path) {
    // Send request for JSON file
    add_uint8_to_buffer(static_cast<uint8_t>(ClientAction::GET_JSON_FILE));
    add_string_to_buffer(file_path);
    send_buffer();

    // Receive JSON content as string
    std::string json_content = recv_string();

    std::cout << "LOADING JSON: " << file_path << "\n";
    std::cout << "JSON SIZE: " << json_content.size() << "\n";

    // Parse and return JSON
    return nlohmann::json::parse(json_content);
}
