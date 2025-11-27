#include "test_client.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../client/client_protocol.h"
#include "../common/client_event_dto.h"
#include "../common/common_protocol.h"
#include "../common/constants.h"
#include "../common/leaderboard_dto.h"
#include "../common/lobby_dto.h"
#include "../common/movement_dto.h"

#include "mock_socket.h"

#define SIZE_UINT32_T 4
#define SIZE_UINT16_T 2
#define SIZE_UINT8_T 1


using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;


void ClientProtocolTest::SetUp() {
    mock_socket = std::make_unique<MockSocket>();
    protocol = std::make_unique<ClientProtocol>(*mock_socket);
}

void ClientProtocolTest::TearDown() {
    mock_socket.reset();
    protocol.reset();
}

// Los TEST_F van FUERA de la clase, en el archivo .cpp

// SEND
TEST_F(ClientProtocolTest, AskForRaceCatalogue_WithMock) {
    uint8_t first_byte = 0;

    EXPECT_CALL(*mock_socket, sendall(::testing::_, 12))
            .WillOnce([&first_byte](const void* data, size_t len) {
                // Capturar solo el primer byte
                first_byte = *static_cast<const uint8_t*>(data);
                return len;
            });

    protocol->ask_for_race_catalogue();

    // Verificar que el primer byte sea el LobbyAction correcto
    EXPECT_EQ(first_byte, static_cast<uint8_t>(LobbyAction::LIST_RACES));
}

TEST_F(ClientProtocolTest, HandleMove_WithMock) {
    MovementDTO movement(Direction::UP, true);

    std::vector<uint8_t> captured_bytes;

    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->handle_move(movement);

    EXPECT_EQ(captured_bytes.size(), 3);

    EXPECT_EQ(captured_bytes[0], static_cast<uint8_t>(ClientAction::SEND_CAR_MOVEMENT));  // action
    EXPECT_EQ(captured_bytes[1], static_cast<uint8_t>(Direction::UP));  // direction
    EXPECT_EQ(captured_bytes[2], 1);                                    // isMoving (true=1)
}

TEST_F(ClientProtocolTest, HandleBuy_WithMock) {
    BuyUpgradeDTO buy_upgrade;
    buy_upgrade.upgrade = ButtonType::Speed;

    std::vector<uint8_t> captured_bytes;

    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->handle_buy(buy_upgrade);

    EXPECT_EQ(captured_bytes.size(), 2);

    EXPECT_EQ(captured_bytes[0], static_cast<uint8_t>(ClientAction::SEND_UPGRADE));  // action
    EXPECT_EQ(captured_bytes[1], static_cast<uint8_t>(ButtonType::Speed));           // upgrade
}

TEST_F(ClientProtocolTest, SendCreateLobby_WithMock) {
    LobbyDTO lobby(LobbyAction::CREATE_LOBBY, 1, "TestLobby", 4, 0);

    std::vector<uint8_t> captured_bytes;

    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->send_create_lobby(lobby);

    // expected size calculation
    size_t expected_size = SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT32_T + lobby.lobby_name.size() +
                           SIZE_UINT8_T + SIZE_UINT8_T + SIZE_UINT8_T;
    EXPECT_EQ(captured_bytes.size(), expected_size);

    // test action
    uint8_t action = captured_bytes[0];
    EXPECT_EQ(action, static_cast<uint8_t>(LobbyAction::CREATE_LOBBY));

    // test lobby id, form bytes 1 to 4
    uint32_t lobby_id = 0;
    for (size_t i = 1; i <= 4; ++i) {
        lobby_id = (lobby_id << 8) | captured_bytes[i];
    }
    EXPECT_EQ(lobby_id, 1);

    // test size lobby name
    // size name is also uint32_t, bytes 5 to 8
    uint32_t name_size = 0;
    for (size_t i = 5; i <= 8; ++i) {
        name_size = (name_size << 8) | captured_bytes[i];
    }
    EXPECT_EQ(name_size, lobby.lobby_name.size());

    // test lobby name, byte 9 to 9 + name_size -1 (17)
    std::string lobby_name(captured_bytes.begin() + 9, captured_bytes.begin() + 9 + name_size);
    EXPECT_EQ(lobby_name, lobby.lobby_name);

    // test max players, byte 18
    uint8_t max_players = captured_bytes[18];
    EXPECT_EQ(max_players, lobby.max_players);

    // test active players, byte 19
    uint8_t active_players = captured_bytes[19];
    EXPECT_EQ(active_players, lobby.active_players);
}

TEST_F(ClientProtocolTest, AskForLobbies_WithMock) {
    LobbyDTO lobby(LobbyAction::SEND_LOBBY_LIST);

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->ask_for_lobbies(lobby);

    // check action send
    size_t expected_size = SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT32_T + SIZE_UINT8_T +
                           SIZE_UINT8_T + SIZE_UINT8_T;
    EXPECT_EQ(captured_bytes.size(), expected_size);  // action + lobby_id + lobby_name size +
                                                      // max_players + active_players + race

    uint8_t action = captured_bytes[0];
    EXPECT_EQ(action, static_cast<uint8_t>(LobbyAction::SEND_LOBBY_LIST));

    // the rest of the bytes should be zero
    for (size_t i = 1; i < captured_bytes.size(); ++i) {
        EXPECT_EQ(captured_bytes[i], 0);
    }
}

TEST_F(ClientProtocolTest, SendCarSelection_WithMock) {
    CarDTO car("TestModel");

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->send_car_selection(car);

    // check size sent
    size_t expected_size = SIZE_UINT8_T + SIZE_UINT32_T +
                           car.model.size();  // action + size of string + string bytes
    EXPECT_EQ(captured_bytes.size(), expected_size);

    // check action send
    uint8_t action = captured_bytes[0];
    EXPECT_EQ(action, static_cast<uint8_t>(ClientAction::SEND_CAR_SELECTION));

    // check car model sent
    // size of string is bytes 1 to 4
    uint32_t model_size = 0;
    for (size_t i = 1; i <= 4; ++i) {
        model_size = (model_size << 8) | captured_bytes[i];
    }
    EXPECT_EQ(model_size, car.model.size());
    // string bytes from byte 5 to end
    std::string model_sent(captured_bytes.begin() + 5, captured_bytes.end());
    EXPECT_EQ(model_sent, car.model);
}

TEST_F(ClientProtocolTest, ChangeUsername_WithMock) {
    std::string username = "Player1";
    LobbyDTO lobby(LobbyAction::CHANGE_USERNAME, username);

    std::vector<uint8_t> captured_bytes;

    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->change_username(lobby);

    // check size sent
    size_t expected_size = SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT32_T + lobby.lobby_name.size() +
                           SIZE_UINT8_T + SIZE_UINT8_T + SIZE_UINT8_T;
    EXPECT_EQ(expected_size, captured_bytes.size());


    // check action sent
    uint8_t action = captured_bytes[0];
    EXPECT_EQ(action, static_cast<uint8_t>(LobbyAction::CHANGE_USERNAME));

    // id should be zero, byte 1 to 4
    uint32_t lobby_id = 0;
    for (size_t i = 1; i <= 4; ++i) {
        lobby_id = (lobby_id << 8) | captured_bytes[i];
    }
    EXPECT_EQ(lobby_id, 0);

    // check size of username sent, byte 5 to 8
    uint32_t name_size = 0;
    for (size_t i = 5; i <= 8; ++i) {
        name_size = (name_size << 8) | captured_bytes[i];
    }
    EXPECT_EQ(name_size, username.size());

    // check username sent, from byte 9 to 9 + name_size -1
    std::string name_sent(captured_bytes.begin() + 9, captured_bytes.begin() + 9 + name_size);
    EXPECT_EQ(name_sent, username);

    // the rest of the bytes should be zero
    for (size_t i = 9 + name_size; i < captured_bytes.size(); ++i) {
        EXPECT_EQ(captured_bytes[i], 0);
    }
}

TEST_F(ClientProtocolTest, SendExit_WithMock) {
    LobbyDTO lobby(LobbyAction::EXIT_GAME);

    std::vector<uint8_t> captured_bytes;

    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->change_username(lobby);

    // check size sent
    size_t expected_size = SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT32_T + SIZE_UINT8_T +
                           SIZE_UINT8_T + SIZE_UINT8_T;
    EXPECT_EQ(expected_size, captured_bytes.size());


    // check action sent
    uint8_t action = captured_bytes[0];
    EXPECT_EQ(action, static_cast<uint8_t>(LobbyAction::EXIT_GAME));

    // the rest should be zero
    for (size_t i = 1; i < captured_bytes.size(); ++i) {
        EXPECT_EQ(captured_bytes[i], 0);
    }
}

TEST_F(ClientProtocolTest, SendRaceSelection_WithMock) {
    RaceDTO race1("Race1", "City1", {}, 3);
    RaceDTO race2("Race2", "City2", {}, 5);
    std::vector<RaceDTO> races = {race1, race2};

    std::vector<uint8_t> captured_bytes;
    EXPECT_CALL(*mock_socket, sendall(::testing::_, ::testing::_))
            .WillOnce([&captured_bytes](const void* data, size_t len) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                captured_bytes.assign(bytes, bytes + len);
                return len;
            });

    protocol->send_races_selection(races);

    // check size sent
    size_t expected_size = SIZE_UINT8_T + SIZE_UINT32_T + SIZE_UINT32_T + SIZE_UINT8_T +
                           SIZE_UINT8_T + SIZE_UINT8_T +
                           (SIZE_UINT32_T + race1.name.size() + SIZE_UINT32_T + race1.city.size() +
                            SIZE_UINT8_T + SIZE_UINT8_T) +
                           (SIZE_UINT32_T + race2.name.size() + SIZE_UINT32_T + race2.city.size() +
                            SIZE_UINT8_T + SIZE_UINT8_T);  // action + size of string + string bytes

    EXPECT_EQ(expected_size, captured_bytes.size());
}


// RECIEVE
TEST_F(ClientProtocolTest, ReceiveCarPosition_WithMock) {
    // Push mock data
    std::vector<uint8_t> mock_data = {
            0x00, 0x00, 0x00, 0x01,  // id = 1
            0x00, 0x00, 0x00, 0x64,  // x = 100
            0x00, 0x00, 0x00, 0xC8,  // y = 200
            0x00, 0x00, 0x00, 0x01,  // angle = 1.0f
            0x00,                    // next checkpoint id = 0
            0x00, 0x00, 0x00, 0x01,  // string
            'H',                     // next checkpoint orientation = 'H'
            0x00, 0x32,              // next checkpoint x = 50
            0x00, 0x64,              // next checkpoint y = 100
            0x0A,                    // next checkpoint width = 10
            0x14,                    // next checkpoint height = 20
            0x01                     // layer = 1
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

    CarPositionDTO car_position = protocol->receive_car_position();

    // Verify received data
    EXPECT_EQ(car_position.client_id, 1);
    EXPECT_EQ(car_position.x_position, 100);
    EXPECT_EQ(car_position.y_position, 200);
    EXPECT_FLOAT_EQ(car_position.angle, 1.0f);
    EXPECT_EQ(car_position.next.id, 0);
    EXPECT_EQ(car_position.next.orientation, 'H');
    EXPECT_EQ(car_position.next.x, 50);
    EXPECT_EQ(car_position.next.y, 100);
    EXPECT_EQ(car_position.next.width, 10);
    EXPECT_EQ(car_position.next.height, 20);
    EXPECT_EQ(car_position.layer, CarLayer::ON_BRIDGE);
}

TEST_F(ClientProtocolTest, ReceiveCarCatalogue_WithMock) {
    // Push mock data
    std::vector<uint8_t> mock_data = {
            0x02,                              // size = 2
            0x00, 0x00, 0x00, 0x06,            // string size = 6
            'M',  'o',  'd',  'e',  'l', '1',  // model = "Model1"
            0x00, 0x64,                        // health = 100
            0x32,                              // handling = 50
            0x64,                              // speed = 100
            0x50,                              // acceleration = 80
            0x00, 0x00, 0x00, 0x06,            // string size = 6
            'M',  'o',  'd',  'e',  'l', '2',  // model = "Model2"
            0x00, 0xC8,                        // health = 200
            0x46,                              // handling = 70
            0x7D,                              // speed = 125
            0x5A                               // acceleration = 90
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

    CarCatalogueDTO car_catalogue = protocol->receive_car_catalogue();

    // Verify received data
    EXPECT_EQ(car_catalogue.size, 2);

    // frist car
    EXPECT_EQ(car_catalogue.cars[0].model, "Model1");
    EXPECT_EQ(car_catalogue.cars[0].health, 100);
    EXPECT_EQ(car_catalogue.cars[0].handling, 50);
    EXPECT_EQ(car_catalogue.cars[0].speed, 100);
    EXPECT_EQ(car_catalogue.cars[0].acceleration, 80);

    // second car
    EXPECT_EQ(car_catalogue.cars[1].model, "Model2");
    EXPECT_EQ(car_catalogue.cars[1].health, 200);
    EXPECT_EQ(car_catalogue.cars[1].handling, 70);
    EXPECT_EQ(car_catalogue.cars[1].speed, 125);
    EXPECT_EQ(car_catalogue.cars[1].acceleration, 90);
}

TEST_F(ClientProtocolTest, ReceiveLobbyCatalogue_WithMock) {
    // push mock data
    std::vector<uint8_t> mock_data = {
            0x02,  // size = 2
            // First lobby
            0x6U,                                         // action = 6 (INFO_LOBBY)
            0x00, 0x00, 0x00, 0x01,                       // lobby_id = 1
            0x00, 0x00, 0x00, 0x09,                       // string size = 9
            'T', 'e', 's', 't', 'L', 'o', 'b', 'b', 'y',  // lobby_name = "TestLobby"
            0x04,                                         // max_players = 4
            0x02,                                         // active_players = 2
            // Second lobby
            0x6U,                    // action = 6 (INFO_LOBBY)
            0x00, 0x00, 0x00, 0x02,  // lobby_id = 2
            0x00, 0x00, 0x00, 0x0C,  // string size = 12
            'A', 'n', 'o', 't', 'h', 'e', 'r', 'L', 'o', 'b', 'b',
            'y',   // lobby_name = "AnotherLobby"
            0x08,  // max_players = 8
            0x05   // active_players = 5
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

    LobbyCatalogueDTO lobby_catalogue = protocol->receive_lobby_catalogue();

    // Verify received data
    EXPECT_EQ(lobby_catalogue.size, 2);

    // First lobby
    EXPECT_EQ(lobby_catalogue.lobbies[0].action, LobbyAction::INFO_LOBBY);
    EXPECT_EQ(lobby_catalogue.lobbies[0].lobby_id, 1);
    EXPECT_EQ(lobby_catalogue.lobbies[0].lobby_name, "TestLobby");
    EXPECT_EQ(lobby_catalogue.lobbies[0].max_players, 4);
    EXPECT_EQ(lobby_catalogue.lobbies[0].active_players, 2);

    // second Lobby
    EXPECT_EQ(lobby_catalogue.lobbies[1].action, LobbyAction::INFO_LOBBY);
    EXPECT_EQ(lobby_catalogue.lobbies[1].lobby_id, 2);
    EXPECT_EQ(lobby_catalogue.lobbies[1].lobby_name, "AnotherLobby");
    EXPECT_EQ(lobby_catalogue.lobbies[1].max_players, 8);
    EXPECT_EQ(lobby_catalogue.lobbies[1].active_players, 5);
}

TEST_F(ClientProtocolTest, ReceiveID_WithMock) {
    // push mock data
    std::vector<uint8_t> mock_data = {
            0x00, 0x00, 0x04, 0xD2  // id = 1234
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

    uint32_t id = protocol->receive_id();

    // Verify received data
    EXPECT_EQ(id, 1234);
}

TEST_F(ClientProtocolTest, ReceiveRace_WithMock) {
    // push mock data
    std::vector<uint8_t> mock_data = {
            0x00, 0x00, 0x00, 0x07,             // string size = 7
            'R', 'a', 'c', 'e', '1', '2', '3',  // name = "Race123"
            0x00, 0x00, 0x00, 0x04,             // string size = 4
            'C', 'i', 't', 'y',                 // city = "City"
            0x02,                               // checkpoints size = 2
            // First checkpoint
            0x01,                    // id = 1
            0x00, 0x00, 0x00, 0x01,  // orientation = 1 ('H')
            'H',                     // orientation = 'H'
            0x00, 0x64,              // x = 100
            0x00, 0xC8,              // y = 200
            0x0A,                    // width = 10
            0x14,                    // height = 20
            // Second checkpoint
            0x02,                    // id = 2
            0x00, 0x00, 0x00, 0x01,  // orientation = 1 ('V')
            'V',                     // orientation = 'V'
            0x01, 0x2C,              // x = 300
            0x01, 0x90,              // y = 400
            0x14,                    // width = 20
            0x1E,                    // height = 30
            0x03                     // laps = 3
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
    RaceDTO race = protocol->receive_race();

    // Verify received data
    EXPECT_EQ(race.name, "Race123");
    EXPECT_EQ(race.city, "City");
    EXPECT_EQ(race.size_checkpoints, 2);
    EXPECT_EQ(race.laps, 3);
    // First checkpoint
    EXPECT_EQ(race.checkpoints[0].id, 1);
    EXPECT_EQ(race.checkpoints[0].orientation, 'H');
    EXPECT_EQ(race.checkpoints[0].x, 100);
    EXPECT_EQ(race.checkpoints[0].y, 200);
    EXPECT_EQ(race.checkpoints[0].width, 10);
    EXPECT_EQ(race.checkpoints[0].height, 20);
    // Second checkpoint
    EXPECT_EQ(race.checkpoints[1].id, 2);
    EXPECT_EQ(race.checkpoints[1].orientation, 'V');
    EXPECT_EQ(race.checkpoints[1].x, 300);
    EXPECT_EQ(race.checkpoints[1].y, 400);
    EXPECT_EQ(race.checkpoints[1].width, 20);
    EXPECT_EQ(race.checkpoints[1].height, 30);
}

TEST_F(ClientProtocolTest, ReceiveUpdatePositions_WithMock) {
    // push mock data
    std::vector<uint8_t> mock_data = {
            0x01,  // size = 1
            // First car position
            // player
            0x00, 0x00, 0x00, 0x07,             // length username
            'P', 'l', 'a', 'y', 'e', 'r', '1',  // username
            0x00, 0x00, 0x00, 0x01,             // id = 1
            0x00, 0x00, 0x00, 0x05,             // length string model
            'M', 'o', 'd', 'e', 'l',            // model
            0x00, 0x64,                         // health = 100
            0x00,                               // flag dead = false
            // car position
            0x00, 0x00, 0x00, 0x01,  // id = 1
            0x00, 0x00, 0x00, 0x64,  // x = 100
            0x00, 0x00, 0x00, 0xC8,  // y = 200
            0x00, 0x00, 0x00, 0x00,  // angle = 0.0f
            0x00,                    // next checkpoint id = 0
            0x00, 0x00, 0x00, 0x01,  // string length = 1
            'H',                     // next checkpoint orientation = 'H'
            0x00, 0x32,              // next checkpoint x = 50
            0x00, 0x64,              // next checkpoint y = 100
            0x0A,                    // next checkpoint width = 10
            0x14,                    // next checkpoint height = 20
            0x01                     // layer = 1
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

    WorldSnapshotDTO world_snapshot = protocol->receive_update_positions();

    // Verify received data
    EXPECT_EQ(world_snapshot.size, 1);
    // First car position
    EXPECT_EQ(world_snapshot.car_positions[0].first.username, "Player1");
    EXPECT_EQ(world_snapshot.car_positions[0].first.id, 1);
    EXPECT_EQ(world_snapshot.car_positions[0].first.car.model, "Model");
    EXPECT_EQ(world_snapshot.car_positions[0].first.health, 100);
    EXPECT_EQ(world_snapshot.car_positions[0].first.dead, 0);
    EXPECT_EQ(world_snapshot.car_positions[0].second.client_id, 1);
    EXPECT_EQ(world_snapshot.car_positions[0].second.x_position, 100);
    EXPECT_EQ(world_snapshot.car_positions[0].second.y_position, 200);
    EXPECT_FLOAT_EQ(world_snapshot.car_positions[0].second.angle, 0.0f);
    EXPECT_EQ(world_snapshot.car_positions[0].second.next.id, 0);
    EXPECT_EQ(world_snapshot.car_positions[0].second.next.orientation, 'H');
    EXPECT_EQ(world_snapshot.car_positions[0].second.next.x, 50);
    EXPECT_EQ(world_snapshot.car_positions[0].second.next.y, 100);
    EXPECT_EQ(world_snapshot.car_positions[0].second.next.width, 10);
    EXPECT_EQ(world_snapshot.car_positions[0].second.next.height, 20);
    EXPECT_EQ(world_snapshot.car_positions[0].second.layer, CarLayer::ON_BRIDGE);
}

TEST_F(ClientProtocolTest, ReceiveLeaderBoard_WithMock) {
    // push mock data
    std::vector<uint8_t> mock_data = {
            0x00, 0x00, 0x00, 0x09,                       // length = 9
            'R', 'a', 'c', 'e', 'n', 'a', 'm', 'e', '1',  // name
            0x01,                                         // size = 1
            // player 1

            0x00, 0x00, 0x00, 0x01,             // id
            0x00, 0x00, 0x00, 0x07,             // size username
            'P', 'l', 'a', 'y', 'e', 'r', '1',  // username
            0x05,                               // position
            0x32,                               // points
            0x00, 0x00, 0x03, 0xE8              // time = 1000
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
    LeaderboardDTO leader_board = protocol->receive_leaderboard();

    // Verify received data
    EXPECT_EQ(leader_board.race_name, "Racename1");
    EXPECT_EQ(leader_board.n_players, 1);
    // player 1
    EXPECT_EQ(leader_board.players[0].client_id, 1);
    EXPECT_EQ(leader_board.players[0].username, "Player1");
    EXPECT_EQ(leader_board.players[0].position, 5);
    EXPECT_EQ(leader_board.players[0].points, 50);
    EXPECT_EQ(leader_board.players[0].total_time, 1000);
}
