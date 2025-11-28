#include "test_server.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../common/car_dto.h"
#include "../common/client_event_dto.h"
#include "../common/common_protocol.h"
#include "../common/constants.h"
#include "../common/leaderboard_dto.h"
#include "../common/lobby_dto.h"
#include "../common/movement_dto.h"
#include "../server/server_protocol.h"

#include "mock_socket.h"

#define SIZE_UINT32_T 4
#define SIZE_UINT16_T 2
#define SIZE_UINT8_T 1


using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;


void ServerProtocolTest::SetUp() {
    mock_socket = std::make_unique<MockSocket>();
    protocol = std::make_unique<ServerProtocol>(*mock_socket);
}

void ServerProtocolTest::TearDown() {
    mock_socket.reset();
    protocol.reset();
}

// SEND
TEST_F(ServerProtocolTest, send_player_id) {
    uint32_t player_id = 123456789;

    std::vector<uint8_t> captured_bytes;

    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->send_player_id(player_id);

    EXPECT_EQ(captured_bytes.size(), 4);
    // check client id bytes equal to client_id
    uint32_t received_id = (static_cast<uint32_t>(captured_bytes[0]) << 24) |
                           (static_cast<uint32_t>(captured_bytes[1]) << 16) |
                           (static_cast<uint32_t>(captured_bytes[2]) << 8) |
                           (static_cast<uint32_t>(captured_bytes[3]));
    EXPECT_EQ(received_id, player_id);
}

TEST_F(ServerProtocolTest, send_car_catalogue) {
    CarDTO car1("ModelX", 200, 100, 150, 100);
    CarDTO car2("ModelY", 250, 100, 160, 100);
    std::vector<CarDTO> cars = {car1, car2};
    CarCatalogueDTO catalogue(cars);

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });
    protocol->send_car_catalogue(catalogue);

    size_t expected_size = SIZE_UINT8_T +
                           (SIZE_UINT32_T + car1.model.size() + SIZE_UINT16_T + SIZE_UINT8_T +
                            SIZE_UINT8_T + SIZE_UINT8_T) +
                           (SIZE_UINT32_T + car2.model.size() + SIZE_UINT16_T + SIZE_UINT8_T +
                            SIZE_UINT8_T + SIZE_UINT8_T);
    EXPECT_EQ(captured_bytes.size(), expected_size);
    // verify sent data
    size_t offset = 0;

    // first byte is size catalogue
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(catalogue.size));
    offset += 1;

    // car1
    // byte 1 to 4 size model
    uint32_t model_size_car1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                               (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                               (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                               (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(model_size_car1, car1.model.size());
    offset += 4;
    // model string, bytes 5 to 5 + model_size -1
    std::string model_car1(captured_bytes.begin() + offset,
                           captured_bytes.begin() + offset + model_size_car1);
    EXPECT_EQ(model_car1, car1.model);
    offset += model_size_car1;
    // health bytes, 2 bytes
    uint16_t health_car1 = (static_cast<uint16_t>(captured_bytes[offset]) << 8) |
                           (static_cast<uint16_t>(captured_bytes[offset + 1]));
    EXPECT_EQ(health_car1, car1.health);
    offset += 2;
    // handling byte
    EXPECT_EQ(captured_bytes[offset], car1.handling);
    offset += 1;
    // speed byte
    EXPECT_EQ(captured_bytes[offset], car1.speed);
    offset += 1;
    // acceleration byte
    EXPECT_EQ(captured_bytes[offset], car1.acceleration);
    offset += 1;

    // car2
    // byte 1 to 4 size model
    uint32_t model_size_car2 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                               (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                               (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                               (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(model_size_car2, car2.model.size());
    offset += 4;
    // model string, bytes 5 to 5 + model_size -1
    std::string model_car2(captured_bytes.begin() + offset,
                           captured_bytes.begin() + offset + model_size_car2);
    EXPECT_EQ(model_car2, car2.model);
    offset += model_size_car2;
    // health bytes, 2 bytes
    uint16_t health_car2 = (static_cast<uint16_t>(captured_bytes[offset]) << 8) |
                           (static_cast<uint16_t>(captured_bytes[offset + 1]));
    EXPECT_EQ(health_car2, car2.health);
    offset += 2;
    // handling byte
    EXPECT_EQ(captured_bytes[offset], car2.handling);
    offset += 1;
    // speed byte
    EXPECT_EQ(captured_bytes[offset], car2.speed);
    offset += 1;
    // acceleration byte
    EXPECT_EQ(captured_bytes[offset], car2.acceleration);
    offset += 1;
}

TEST_F(ServerProtocolTest, SendWorldSnapshot) {
    WorldSnapshotDTO world_snapshot;
    PlayerDTO player1("player1", 1001, CarDTO("ModelX"), 200, false);
    CarPositionDTO car_position1(100, 150, 90, 200, CheckpointDTO(1, 'H', 1001, 1001, 20, 30),
                                 CarLayer::ON_GROUND);
    world_snapshot.add_car_position(player1, car_position1);
    GameUpdateDTO snapshot(GameLoopAction::SEND_UPDATE_POSITIONS, world_snapshot);

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->send_world_snapshot(snapshot);

    // calculate expected size
    size_t expected_size = SIZE_UINT8_T +  // gameloop action
                           SIZE_UINT8_T +  // ws size
                           SIZE_UINT32_T + player1.username.size() + SIZE_UINT32_T + SIZE_UINT32_T +
                           player1.car.model.size() + SIZE_UINT16_T + SIZE_UINT8_T +  // player1
                           4 * SIZE_UINT32_T + SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT8_T +
                           2 * SIZE_UINT16_T + 3 * SIZE_UINT8_T + // car position1
                           SIZE_UINT8_T;  //NPC size (0)
    EXPECT_EQ(captured_bytes.size(), expected_size);

    // verify_data
    size_t offset = 0;
    // gameloop action
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(snapshot.gameloop_action));
    offset += 1;
    // world snapshot size
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(world_snapshot.size));
    offset += 1;
    // player1
    // username size bytes
    uint32_t username_size = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                             (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                             (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                             (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(username_size, player1.username.size());
    offset += 4;
    // username string
    std::string username(captured_bytes.begin() + offset,
                         captured_bytes.begin() + offset + username_size);
    EXPECT_EQ(username, player1.username);
    offset += username_size;
    // client id bytes
    uint32_t client_id = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                         (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                         (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                         (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(client_id, player1.id);
    offset += 4;
    // car model size bytes
    uint32_t car_model_size = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                              (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                              (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                              (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(car_model_size, player1.car.model.size());
    offset += 4;
    // car model string
    std::string car_model(captured_bytes.begin() + offset,
                          captured_bytes.begin() + offset + car_model_size);
    EXPECT_EQ(car_model, player1.car.model);
    offset += car_model_size;
    // car health bytes
    uint16_t car_health = (static_cast<uint16_t>(captured_bytes[offset]) << 8) |
                          (static_cast<uint16_t>(captured_bytes[offset + 1]));
    EXPECT_EQ(car_health, player1.health);
    offset += 2;
    // player dead byte
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(player1.dead));
    offset += 1;
    // car position1
    // client id bytes
    uint32_t cp_client_id = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                            (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                            (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                            (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(cp_client_id, car_position1.client_id);
    offset += 4;
    // x bytes
    uint32_t x_pos = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                     (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                     (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                     (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(x_pos, car_position1.x_position);
    offset += 4;
    // y bytes
    uint32_t y_pos = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                     (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                     (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                     (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(y_pos, car_position1.y_position);
    offset += 4;
    // angle bytes
    uint32_t angle = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                     (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                     (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                     (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(angle, car_position1.angle);
    offset += 4;
    // next checkpoint id byte
    EXPECT_EQ(captured_bytes[offset], car_position1.next.id);
    offset += 1;
    // next checkpoint size bytes
    uint32_t next_cp_size = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                            (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                            (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                            (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(next_cp_size, 1);
    offset += 4;
    // next checkpoint orientation byte
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(car_position1.next.orientation));
    offset += 1;
    // next checkpoint x bytes
    uint16_t next_x = (static_cast<uint16_t>(captured_bytes[offset]) << 8) |
                      (static_cast<uint16_t>(captured_bytes[offset + 1]));
    EXPECT_EQ(next_x, car_position1.next.x);
    offset += 2;
    // next checkpoint y bytes
    uint16_t next_y = (static_cast<uint16_t>(captured_bytes[offset]) << 8) |
                      (static_cast<uint16_t>(captured_bytes[offset + 1]));
    EXPECT_EQ(next_y, car_position1.next.y);
    offset += 2;
    // next checkpoint width byte
    EXPECT_EQ(captured_bytes[offset], car_position1.next.width);
    offset += 1;
    // next checkpoint height byte
    EXPECT_EQ(captured_bytes[offset], car_position1.next.height);
    offset += 1;
    // car layer byte
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(car_position1.layer));
}

TEST_F(ServerProtocolTest, SendRaceUpdate) {
    RaceDTO race("Grand Prix", "Monaco", {}, 3);
    GameUpdateDTO snapshot(GameLoopAction::SEND_RACE, race);

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });
    protocol->send_race_update(snapshot);

    // calculate expected size
    size_t expected_size = SIZE_UINT8_T +  // gameloop action
                           SIZE_UINT32_T + race.name.size() + SIZE_UINT32_T + race.city.size() +
                           SIZE_UINT8_T + SIZE_UINT8_T;
    EXPECT_EQ(captured_bytes.size(), expected_size);

    // verify data
    size_t offset = 0;
    // gameloop action
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(snapshot.gameloop_action));
    offset += 1;
    // size race name
    uint32_t name_size = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                         (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                         (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                         (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(name_size, race.name.size());
    offset += 4;
    // race name string
    std::string name(captured_bytes.begin() + offset, captured_bytes.begin() + offset + name_size);
    EXPECT_EQ(name, race.name);
    offset += name_size;
    // size race city
    uint32_t city_size = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                         (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                         (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                         (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(city_size, race.city.size());
    offset += 4;
    // race city string
    std::string city(captured_bytes.begin() + offset, captured_bytes.begin() + offset + city_size);
    EXPECT_EQ(city, race.city);
    offset += city_size;
    // size checkpoints byte
    EXPECT_EQ(captured_bytes[offset], race.size_checkpoints);
    offset += 1;
    // laps byte
    EXPECT_EQ(captured_bytes[offset], race.laps);
}

TEST_F(ServerProtocolTest, SendLeaderboardUpdate) {
    PlayerRaceStatsDTO player1(1001, "player1", 1, 10, 30000);
    PlayerRaceStatsDTO player2(1002, "player2", 2, 8, 32000);
    std::vector<PlayerRaceStatsDTO> players = {player1, player2};
    LeaderboardDTO leaderboard("Grand Prix", players);
    GameUpdateDTO snapshot(GameLoopAction::SEND_LEADERBOARD, leaderboard);

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });
    protocol->send_leaderboard_update(snapshot);

    // calculate expected size
    size_t expected_size =
            SIZE_UINT8_T +  // gameloop action
            SIZE_UINT32_T + leaderboard.race_name.size() + SIZE_UINT8_T +
            (2 * SIZE_UINT32_T + player1.username.size() + 2 * SIZE_UINT8_T + SIZE_UINT32_T) +
            (2 * SIZE_UINT32_T + player2.username.size() + 2 * SIZE_UINT8_T + SIZE_UINT32_T);
    EXPECT_EQ(captured_bytes.size(), expected_size);

    // verify data
    size_t offset = 0;
    // gameloop action
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(snapshot.gameloop_action));
    offset += 1;
    // race name size
    uint32_t race_name_size = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                              (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                              (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                              (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(race_name_size, leaderboard.race_name.size());
    offset += 4;
    // race name string
    std::string race_name(captured_bytes.begin() + offset,
                          captured_bytes.begin() + offset + race_name_size);
    EXPECT_EQ(race_name, leaderboard.race_name);
    offset += race_name_size;
    // number of players byte
    EXPECT_EQ(captured_bytes[offset], leaderboard.n_players);
    offset += 1;

    // player1
    // client id bytes
    uint32_t client_id1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                          (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                          (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                          (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(client_id1, player1.client_id);
    offset += 4;
    // username size
    uint32_t username_size1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                              (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                              (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                              (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(username_size1, player1.username.size());
    offset += 4;
    // username string
    std::string username1(captured_bytes.begin() + offset,
                          captured_bytes.begin() + offset + username_size1);
    EXPECT_EQ(username1, player1.username);
    offset += username_size1;
    // position byte
    EXPECT_EQ(captured_bytes[offset], player1.position);
    offset += 1;
    // points byte
    EXPECT_EQ(captured_bytes[offset], player1.points);
    offset += 1;
    // total time bytes
    uint32_t total_time1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                           (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                           (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                           (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(total_time1, player1.total_time);
    offset += 4;
}

TEST_F(ServerProtocolTest, SendLobbyCatalogue) {
    LobbyDTO lobby1(LobbyAction::INFO_LOBBY, 1001, "LobbyOne", 4, 2);
    LobbyDTO lobby2(LobbyAction::INFO_LOBBY, 1002, "LobbyTwo", 6, 5);
    std::vector<LobbyDTO> lobbies = {lobby1, lobby2};
    LobbyCatalogueDTO lobby_catalogue(lobbies);

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });
    protocol->send_lobby_catalogue(lobby_catalogue);

    // calculate expected size
    size_t expected_size = SIZE_UINT8_T +  // size catalogue
                           (SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT32_T +
                            lobby1.lobby_name.size() + 2 * SIZE_UINT8_T) +
                           (SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT32_T +
                            lobby2.lobby_name.size() + 2 * SIZE_UINT8_T);
    EXPECT_EQ(captured_bytes.size(), expected_size);

    // verify data
    size_t offset = 0;
    // size catalogue byte
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(lobby_catalogue.size));
    offset += 1;
    // lobby1
    // action byte
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(lobby1.action));
    offset += 1;
    // lobby id bytes
    uint32_t lobby_id1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                         (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                         (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                         (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(lobby_id1, lobby1.lobby_id);
    offset += 4;
    // lobby name size bytes
    uint32_t lobby_name_size1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                                (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                                (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                                (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(lobby_name_size1, lobby1.lobby_name.size());
    offset += 4;
    // lobby name string
    std::string lobby_name1(captured_bytes.begin() + offset,
                            captured_bytes.begin() + offset + lobby_name_size1);
    EXPECT_EQ(lobby_name1, lobby1.lobby_name);
    offset += lobby_name_size1;
    // max players byte
    EXPECT_EQ(captured_bytes[offset], lobby1.max_players);
    offset += 1;
    // active players byte
    EXPECT_EQ(captured_bytes[offset], lobby1.active_players);
    offset += 1;
    // lobby2
    // action byte
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(lobby2.action));
    offset += 1;
    // lobby id bytes
    uint32_t lobby_id2 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                         (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                         (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                         (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(lobby_id2, lobby2.lobby_id);
    offset += 4;
    // lobby name size bytes
    uint32_t lobby_name_size2 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                                (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                                (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                                (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(lobby_name_size2, lobby2.lobby_name.size());
    offset += 4;
    // lobby name string
    std::string lobby_name2(captured_bytes.begin() + offset,
                            captured_bytes.begin() + offset + lobby_name_size2);
    EXPECT_EQ(lobby_name2, lobby2.lobby_name);
    offset += lobby_name_size2;
    // max players byte
    EXPECT_EQ(captured_bytes[offset], lobby2.max_players);
    offset += 1;
    // active players byte
    EXPECT_EQ(captured_bytes[offset], lobby2.active_players);
    offset += 1;
}

TEST_F(ServerProtocolTest, SendRaceCatalogue) {
    RaceDTO race1("Grand Prix", "Monaco", {}, 3);
    RaceDTO race2("Speedway", "Daytona", {}, 5);
    std::vector<RaceDTO> races = {race1, race2};
    RaceCatalogueDTO race_catalogue(races);

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });
    protocol->send_races_catalogue(race_catalogue);

    // calculate expected size
    size_t expected_size = SIZE_UINT8_T +  // size catalogue
                           (SIZE_UINT32_T + race1.name.size() + SIZE_UINT32_T + race1.city.size() +
                            SIZE_UINT8_T + SIZE_UINT8_T) +
                           (SIZE_UINT32_T + race2.name.size() + SIZE_UINT32_T + race2.city.size() +
                            SIZE_UINT8_T + SIZE_UINT8_T);
    EXPECT_EQ(captured_bytes.size(), expected_size);

    // verify data
    size_t offset = 0;
    // size catalogue byte
    EXPECT_EQ(captured_bytes[offset], static_cast<uint8_t>(race_catalogue.size));
    offset += 1;
    // race1
    // race name size bytes
    uint32_t race_name_size1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                               (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                               (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                               (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(race_name_size1, race1.name.size());
    offset += 4;
    // race name string
    std::string race_name1(captured_bytes.begin() + offset,
                           captured_bytes.begin() + offset + race_name_size1);
    EXPECT_EQ(race_name1, race1.name);
    offset += race_name_size1;
    // race city size bytes
    uint32_t race_city_size1 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                               (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                               (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                               (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(race_city_size1, race1.city.size());
    offset += 4;
    // race city string
    std::string race_city1(captured_bytes.begin() + offset,
                           captured_bytes.begin() + offset + race_city_size1);
    EXPECT_EQ(race_city1, race1.city);
    offset += race_city_size1;
    // size checkpoints byte
    EXPECT_EQ(captured_bytes[offset], race1.size_checkpoints);
    offset += 1;
    // laps byte
    EXPECT_EQ(captured_bytes[offset], race1.laps);
    offset += 1;
    // race2
    // race name size bytes
    uint32_t race_name_size2 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                               (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                               (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                               (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(race_name_size2, race2.name.size());
    offset += 4;
    // race name string
    std::string race_name2(captured_bytes.begin() + offset,
                           captured_bytes.begin() + offset + race_name_size2);
    EXPECT_EQ(race_name2, race2.name);
    offset += race_name_size2;
    // race city size bytes
    uint32_t race_city_size2 = (static_cast<uint32_t>(captured_bytes[offset]) << 24) |
                               (static_cast<uint32_t>(captured_bytes[offset + 1]) << 16) |
                               (static_cast<uint32_t>(captured_bytes[offset + 2]) << 8) |
                               (static_cast<uint32_t>(captured_bytes[offset + 3]));
    EXPECT_EQ(race_city_size2, race2.city.size());
    offset += 4;
    // race city string
    std::string race_city2(captured_bytes.begin() + offset,
                           captured_bytes.begin() + offset + race_city_size2);
    EXPECT_EQ(race_city2, race2.city);
    offset += race_city_size2;
    // size checkpoints byte
    EXPECT_EQ(captured_bytes[offset], race2.size_checkpoints);
    offset += 1;
    // laps byte
    EXPECT_EQ(captured_bytes[offset], race2.laps);
    offset += 1;
}

// RECEIVE
TEST_F(ServerProtocolTest, ReceiveLobbyInfo) {
    // push mock data
    std::vector<uint8_t> mock_data{
            0x00,                                         // action CreateLobby
            0x00, 0x00, 0x03, 0xE9,                       // lobby id 1001
            0x00, 0x00, 0x00, 0x09,                       // lobby name size 9
            'T', 'e', 's', 't', 'L', 'o', 'b', 'b', 'y',  // lobby name "TestLobby"
            0x04,                                         // max players 4
            0x02,                                         // active players 2
            // races selected
            0x01,                          // size
            0x00, 0x00, 0x00, 0x06,        // race name size
            'M', 'o', 'n', 'a', 'c', 'o',  // race name "Monaco"
            0x00, 0x00, 0x00, 0x06,        // race city size
            'M', 'o', 'n', 'a', 'c', 'o',  // race city "Monaco"
            0x00,                          // size checkpoints
            0x02                           // laps
    };

    auto mock_stream = std::make_shared<std::vector<uint8_t>>(mock_data);
    size_t read_pos = 0;

    EXPECT_CALL(*mock_socket, recvall(::testing::_, ::testing::_))
            .WillRepeatedly([mock_stream, &read_pos](void* buffer, size_t len) -> int {
                if (read_pos >= mock_stream->size()) {
                    return 0;  // EOF
                }
                size_t bytes_to_read = std::min(len, mock_stream->size() - read_pos);
                std::memcpy(buffer, mock_stream->data() + read_pos, bytes_to_read);
                read_pos += bytes_to_read;
                return static_cast<int>(bytes_to_read);
            });

    LobbyDTO lobby = protocol->receive_lobby_info();
    EXPECT_EQ(lobby.action, LobbyAction::CREATE_LOBBY);
    EXPECT_EQ(lobby.lobby_id, 1001);
    EXPECT_EQ(lobby.lobby_name, "TestLobby");
    EXPECT_EQ(lobby.max_players, 4);
    EXPECT_EQ(lobby.active_players, 2);
    EXPECT_EQ(lobby.selected_races.races[0].name, "Monaco");
    EXPECT_EQ(lobby.selected_races.races[0].city, "Monaco");
    EXPECT_EQ(lobby.selected_races.races[0].size_checkpoints, 0);
    EXPECT_EQ(lobby.selected_races.races[0].laps, 2);
}

TEST_F(ServerProtocolTest, ReceiveUsername) {
    // push mock data
    std::vector<uint8_t> mock_data{
            0x01,                                  // event type SetUsername
            0x00, 0x00, 0x00, 0x07,                // username size 7
            'P',  'l',  'a',  'y',  'e', 'r', '1'  // username "Player1"
    };

    auto mock_stream = std::make_shared<std::vector<uint8_t>>(mock_data);
    size_t read_pos = 0;

    EXPECT_CALL(*mock_socket, recvall(::testing::_, ::testing::_))
            .WillRepeatedly([mock_stream, &read_pos](void* buffer, size_t len) -> int {
                if (read_pos >= mock_stream->size()) {
                    return 0;  // EOF
                }
                size_t bytes_to_read = std::min(len, mock_stream->size() - read_pos);
                std::memcpy(buffer, mock_stream->data() + read_pos, bytes_to_read);
                read_pos += bytes_to_read;
                return static_cast<int>(bytes_to_read);
            });

    GameCommandDTO game_command = protocol->receive_game_command();
    // command_data has to be PlayerDTO
    PlayerDTO* event = std::get_if<PlayerDTO>(&game_command.command_data);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->username, "Player1");
}

TEST_F(ServerProtocolTest, ReceiveMovment) {
    // push mock data
    std::vector<uint8_t> mock_data{
            0x03,  // event type Movement
            0x01,  // direction
            0x01,  // is_moving flag = true
    };

    auto mock_stream = std::make_shared<std::vector<uint8_t>>(mock_data);
    size_t read_pos = 0;

    EXPECT_CALL(*mock_socket, recvall(::testing::_, ::testing::_))
            .WillRepeatedly([mock_stream, &read_pos](void* buffer, size_t len) -> int {
                if (read_pos >= mock_stream->size()) {
                    return 0;  // EOF
                }
                size_t bytes_to_read = std::min(len, mock_stream->size() - read_pos);
                std::memcpy(buffer, mock_stream->data() + read_pos, bytes_to_read);
                read_pos += bytes_to_read;
                return static_cast<int>(bytes_to_read);
            });

    GameCommandDTO game_command = protocol->receive_game_command();
    // command_data has to be MovementDTO
    MovementDTO* event = std::get_if<MovementDTO>(&game_command.command_data);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->direction, Direction::UP);
    EXPECT_EQ(event->isMoving, true);
}

TEST_F(ServerProtocolTest, ReceiveCarSelection) {
    // push mock data
    std::vector<uint8_t> mock_data{
            0x05,                             // event type CarSelection
            0x00, 0x00, 0x00, 0x06,           // car model size 6
            'M',  'o',  'd',  'e',  'l', 'X'  // car model "ModelX"
    };

    auto mock_stream = std::make_shared<std::vector<uint8_t>>(mock_data);
    size_t read_pos = 0;

    EXPECT_CALL(*mock_socket, recvall(::testing::_, ::testing::_))
            .WillRepeatedly([mock_stream, &read_pos](void* buffer, size_t len) -> int {
                if (read_pos >= mock_stream->size()) {
                    return 0;  // EOF
                }
                size_t bytes_to_read = std::min(len, mock_stream->size() - read_pos);
                std::memcpy(buffer, mock_stream->data() + read_pos, bytes_to_read);
                read_pos += bytes_to_read;
                return static_cast<int>(bytes_to_read);
            });

    GameCommandDTO game_command = protocol->receive_game_command();
    // command_data has to be CarSelectionDTO
    CarDTO* event = std::get_if<CarDTO>(&game_command.command_data);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->model, "ModelX");
}

TEST_F(ServerProtocolTest, ReceiveUpgrade) {
    // push mock data
    std::vector<uint8_t> mock_data{
            0x07,  // event type Upgrade
            0x01,  // upgrade type Speed
    };

    auto mock_stream = std::make_shared<std::vector<uint8_t>>(mock_data);
    size_t read_pos = 0;

    EXPECT_CALL(*mock_socket, recvall(::testing::_, ::testing::_))
            .WillRepeatedly([mock_stream, &read_pos](void* buffer, size_t len) -> int {
                if (read_pos >= mock_stream->size()) {
                    return 0;  // EOF
                }
                size_t bytes_to_read = std::min(len, mock_stream->size() - read_pos);
                std::memcpy(buffer, mock_stream->data() + read_pos, bytes_to_read);
                read_pos += bytes_to_read;
                return static_cast<int>(bytes_to_read);
            });

    GameCommandDTO game_command = protocol->receive_game_command();
    // command_data has to be BuyUpgradeDTO
    BuyUpgradeDTO* event = std::get_if<BuyUpgradeDTO>(&game_command.command_data);
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->upgrade, ButtonType::Speed);
}

TEST_F(ServerProtocolTest, ReceiveDisconnect) {
    // push mock data
    std::vector<uint8_t> mock_data{
            0x00,  // event type Disconnect
    };

    auto mock_stream = std::make_shared<std::vector<uint8_t>>(mock_data);
    size_t read_pos = 0;

    EXPECT_CALL(*mock_socket, recvall(::testing::_, ::testing::_))
            .WillRepeatedly([mock_stream, &read_pos](void* buffer, size_t len) -> int {
                if (read_pos >= mock_stream->size()) {
                    return 0;  // EOF
                }
                size_t bytes_to_read = std::min(len, mock_stream->size() - read_pos);
                std::memcpy(buffer, mock_stream->data() + read_pos, bytes_to_read);
                read_pos += bytes_to_read;
                return static_cast<int>(bytes_to_read);
            });

    GameCommandDTO game_command = protocol->receive_game_command();
    // command action has to be Disconnect
    EXPECT_EQ(game_command.client_action, ClientAction::DISCONNECT);
}
