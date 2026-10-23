#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../client/client_protocol.h"
#include "../common/socket.h"
#include "../server/server_constants.h"

#include "server_test_utils.h"

// End to end tests: a real acceptor listens on localhost and the tests talk to it through
// sockets using the client protocol, as the client program does.

namespace {

std::atomic<int> next_port{24500};

struct TestClient {
    Socket socket;
    ClientProtocol protocol;

    explicit TestClient(const std::string& port):
            socket("localhost", port.c_str()), protocol(socket) {}

    uint32_t change_username(const std::string& username) {
        protocol.change_username(LobbyDTO(LobbyAction::CHANGE_USERNAME, 0, username));
        return protocol.receive_id();
    }

    LobbyDTO create_game(uint8_t max_players, const std::string& name = "game") {
        protocol.send_create_lobby(LobbyDTO(LobbyAction::CREATE_LOBBY, 0, name, max_players));
        return protocol.receive_lobby();
    }

    LobbyDTO join_game(uint32_t game_id) {
        protocol.send_action_lobby(LobbyDTO(LobbyAction::JOIN_LOBBY, game_id));
        return protocol.receive_lobby();
    }

    LobbyCatalogueDTO list_games() {
        protocol.ask_for_lobbies(LobbyDTO(LobbyAction::SEND_LOBBY_LIST));
        return protocol.receive_lobby_catalogue();
    }

    // Reads updates until one with `action` arrives (the server keeps sending while racing)
    GameUpdateDTO receive_until(GameLoopAction action) {
        GameUpdateDTO update = protocol.receive_update();
        while (update.gameloop_action != action) update = protocol.receive_update();
        return update;
    }

    // What the client does when the server sends it back to the lobby
    void return_to_lobby() {
        receive_until(GameLoopAction::RETURN_TO_LOBBY);
        protocol.send_client_action(ClientAction::DISCONNECT);
    }

    // Leaves the lobby and waits until the server closes the connection. Stopping the server
    // while a handler is still answering races on its protocol buffer (ClientHandler::stop
    // sends from the acceptor thread), so every client leaves before the server stops.
    void leave() {
        protocol.send_exit(LobbyDTO(LobbyAction::LEAVE_LOBBY));
        char byte;
        while (socket.recvsome(&byte, 1) > 0) {}
    }

    ~TestClient() {
        try {
            leave();
        } catch (const std::exception&) {}
    }
};

// Selects `race`, creates a game and consumes the car catalogue
void host_game(TestClient& host, const RaceDTO& race, uint8_t max_players,
               const std::string& name = "game") {
    host.protocol.send_races_selection({race});
    ASSERT_EQ(host.create_game(max_players, name).action, LobbyAction::CREATE_OR_JOIN_SUCCESS);
    ASSERT_EQ(host.protocol.receive_car_catalogue().cars.size(), 7u);
}

class ServerIntegrationTest: public ::testing::Test {
protected:
    ServerConfig config = make_fast_game_config();
    std::string port = std::to_string(next_port++);
    RunningServer server{port, config};
    RaceDTO race = make_test_race(3, 1, "integration_race");

    std::unique_ptr<TestClient> connect() { return std::make_unique<TestClient>(port); }

    // Selects the test race and creates a game, consuming the car catalogue
    void host_game(TestClient& host, uint8_t max_players, const std::string& name = "game") {
        ::host_game(host, race, max_players, name);
    }
};

// Real server with the game loop milestone Background (scenario 3.8): its own config, a fake
// generator that always gives Nitro and a race with 2 power-ups
class ServerIntegrationPowerUpTest: public ::testing::Test {
protected:
    static constexpr int MAX_UPDATES = 4000;

    ServerConfig config = make_fast_power_up_game_config();
    std::string port = std::to_string(next_port++);
    RunningServer server{port, config, [] {
                             return make_sequence_power_up_type_generator({PowerUpType::Nitro});
                         }};
    RaceDTO race = make_test_race(4, 1, "power_up_race");

    std::unique_ptr<TestClient> connect() { return std::make_unique<TestClient>(port); }
};

}  // namespace


TEST_F(ServerIntegrationTest, EveryClientGetsItsOwnId) {
    auto first = connect();
    auto second = connect();

    EXPECT_EQ(first->change_username("first"), FIRST_CLIENT_ID);
    EXPECT_EQ(second->change_username("second"), FIRST_CLIENT_ID + 1);
}

TEST_F(ServerIntegrationTest, ListsTheAvailableRaces) {
    auto client = connect();

    client->protocol.ask_for_race_catalogue();
    RaceCatalogueDTO races = client->protocol.receive_race_catalogue();

    EXPECT_EQ(races.races.size(), 6u);
    for (const auto& each: races.races) EXPECT_FALSE(each.checkpoints.empty()) << each.name;
}

TEST_F(ServerIntegrationTest, RejectsInvalidPlayerCounts) {
    auto client = connect();

    LobbyDTO too_few = client->create_game(0);
    EXPECT_EQ(too_few.action, LobbyAction::MIN_PLAYERS_ERROR);
    EXPECT_EQ(too_few.max_players, config.server.min_players_per_game);

    LobbyDTO too_many = client->create_game(config.server.max_players_per_game + 1);
    EXPECT_EQ(too_many.action, LobbyAction::MAX_PLAYERS_ERROR);
    EXPECT_EQ(too_many.max_players, config.server.max_players_per_game);

    EXPECT_EQ(client->list_games().size, 0);
}

TEST_F(ServerIntegrationTest, JoiningAnUnknownGameFails) {
    auto client = connect();

    EXPECT_EQ(client->join_game(999).action, LobbyAction::CREATE_OR_JOIN_ERROR);
}

TEST_F(ServerIntegrationTest, CreatingTooManyGamesFails) {
    std::vector<std::unique_ptr<TestClient>> hosts;
    for (uint32_t i = 0; i < config.server.max_concurrent_games; ++i) {
        hosts.push_back(connect());
        host_game(*hosts.back(), 2);
    }

    auto late = connect();
    EXPECT_EQ(late->create_game(2).action, LobbyAction::CREATE_OR_JOIN_ERROR);
    EXPECT_EQ(late->list_games().size, config.server.max_concurrent_games);

    // Leave the games so the handlers are not blocked waiting for players
    for (auto& host: hosts) {
        host->protocol.send_client_action(ClientAction::DISCONNECT);
        host->receive_until(GameLoopAction::RETURN_TO_LOBBY);
    }
}

TEST_F(ServerIntegrationTest, SinglePlayerPlaysAFullGame) {
    auto client = connect();
    ASSERT_EQ(client->change_username("mateo"), FIRST_CLIENT_ID);
    host_game(*client, 1);

    client->protocol.send_car_selection(CarDTO("Sport"));

    GameUpdateDTO shop = client->receive_until(GameLoopAction::START_SHOP);
    EXPECT_EQ(std::get<uint8_t>(shop.data), config.game.penalty_per_upgrade);

    GameUpdateDTO race_update = client->receive_until(GameLoopAction::SEND_RACE);
    const RaceDTO& received_race = std::get<RaceDTO>(race_update.data);
    EXPECT_EQ(received_race.name, race.name);
    EXPECT_EQ(received_race.checkpoints.size(), race.checkpoints.size());

    GameUpdateDTO positions = client->receive_until(GameLoopAction::SEND_UPDATE_POSITIONS);
    const WorldSnapshotDTO& snapshot = std::get<WorldSnapshotDTO>(positions.data);
    ASSERT_EQ(snapshot.car_positions.size(), 1u);
    EXPECT_EQ(snapshot.car_positions[0].first.username, "mateo");
    EXPECT_EQ(snapshot.car_positions[0].first.car.model, "Sport");

    client->receive_until(GameLoopAction::START_RACE);
    client->protocol.send_event(MovementDTO(Direction::UP, true));
    client->protocol.send_event(ClientActionDTO(ClientAction::CHEAT_INFINITE_HEALTH));
    client->receive_until(GameLoopAction::RACE_TIME);
    client->protocol.send_event(ClientActionDTO(ClientAction::CHEAT_WIN_RACE));

    GameUpdateDTO leaderboard = client->receive_until(GameLoopAction::SEND_LEADERBOARD);
    const LeaderboardDTO& race_leaderboard = std::get<LeaderboardDTO>(leaderboard.data);
    EXPECT_EQ(race_leaderboard.race_name, race.name);
    ASSERT_EQ(race_leaderboard.players.size(), 1u);
    EXPECT_EQ(race_leaderboard.players[0].username, "mateo");
    EXPECT_EQ(race_leaderboard.players[0].points, 8);

    client->receive_until(GameLoopAction::GAME_FINISHED);
    GameUpdateDTO final_update = client->receive_until(GameLoopAction::SEND_LEADERBOARD);
    EXPECT_EQ(std::get<LeaderboardDTO>(final_update.data).race_name, "Final Leaderboard");

    client->return_to_lobby();

    // Back in the lobby: the handler answers lobby requests again and the game is gone
    EXPECT_EQ(client->change_username("mateo"), FIRST_CLIENT_ID);
    EXPECT_EQ(client->list_games().size, 0);
}

TEST_F(ServerIntegrationTest, TwoPlayersShareAGame) {
    auto host = connect();
    auto guest = connect();
    ASSERT_EQ(host->change_username("host"), FIRST_CLIENT_ID);
    ASSERT_EQ(guest->change_username("guest"), FIRST_CLIENT_ID + 1);
    host_game(*host, 2, "shared");

    LobbyCatalogueDTO games = guest->list_games();
    ASSERT_EQ(games.lobbies.size(), 1u);
    EXPECT_EQ(games.lobbies[0].lobby_id, FIRST_CLIENT_ID);
    EXPECT_EQ(games.lobbies[0].lobby_name, "shared");
    EXPECT_EQ(games.lobbies[0].max_players, 2);
    EXPECT_EQ(games.lobbies[0].active_players, 1);

    ASSERT_EQ(guest->join_game(FIRST_CLIENT_ID).action, LobbyAction::CREATE_OR_JOIN_SUCCESS);
    ASSERT_EQ(guest->protocol.receive_car_catalogue().cars.size(), 7u);

    host->protocol.send_car_selection(CarDTO("Jeep"));
    guest->protocol.send_car_selection(CarDTO("Truck"));

    GameUpdateDTO positions = guest->receive_until(GameLoopAction::SEND_UPDATE_POSITIONS);
    EXPECT_EQ(std::get<WorldSnapshotDTO>(positions.data).car_positions.size(), 2u);

    host->receive_until(GameLoopAction::START_RACE);
    host->protocol.send_client_action(ClientAction::CHEAT_WIN_GAME);

    for (TestClient* client: {host.get(), guest.get()}) {
        client->receive_until(GameLoopAction::GAME_FINISHED);
        GameUpdateDTO final_update = client->receive_until(GameLoopAction::SEND_LEADERBOARD);
        const LeaderboardDTO& leaderboard = std::get<LeaderboardDTO>(final_update.data);
        ASSERT_EQ(leaderboard.players.size(), 2u);
        EXPECT_EQ(leaderboard.players[0].username, "host");
        EXPECT_EQ(leaderboard.players[1].username, "guest");
        client->return_to_lobby();
    }

    EXPECT_EQ(host->change_username("host"), FIRST_CLIENT_ID);
    EXPECT_EQ(guest->change_username("guest"), FIRST_CLIENT_ID + 1);
}

TEST_F(ServerIntegrationTest, LeavingTheGameReturnsToTheLobby) {
    auto client = connect();
    host_game(*client, 2);

    client->protocol.send_client_action(ClientAction::DISCONNECT);
    client->receive_until(GameLoopAction::RETURN_TO_LOBBY);

    EXPECT_EQ(client->change_username("again"), FIRST_CLIENT_ID);
    EXPECT_EQ(client->list_games().size, 0);
}

TEST_F(ServerIntegrationTest, LeavingTheLobbyClosesTheConnection) {
    auto client = connect();
    ASSERT_EQ(client->change_username("bye"), FIRST_CLIENT_ID);

    client->protocol.send_exit(LobbyDTO(LobbyAction::LEAVE_LOBBY));

    EXPECT_ANY_THROW(client->protocol.receive_id());
}

TEST_F(ServerIntegrationTest, ServerKeepsAcceptingAfterAClientLeaves) {
    {
        auto client = connect();
        ASSERT_EQ(client->change_username("first"), FIRST_CLIENT_ID);
    }

    auto client = connect();
    EXPECT_EQ(client->change_username("second"), FIRST_CLIENT_ID + 1);
}


TEST_F(ServerIntegrationPowerUpTest, ClientReceivesCollectedPowerUpInSnapshot) {
    auto client = connect();
    const uint32_t player_id = client->change_username("racer");
    host_game(*client, race, 1);
    client->protocol.send_car_selection(CarDTO("Compact"));
    client->receive_until(GameLoopAction::START_RACE);

    client->protocol.send_event(MovementDTO(Direction::UP, true));

    // Reads snapshots until the power-up shows up inactive or the race ends
    std::optional<WorldSnapshotDTO> collected;
    bool race_ended = false;
    GameUpdateDTO update = client->protocol.receive_update();
    for (int i = 0; i < MAX_UPDATES && !collected; ++i) {
        if (update.gameloop_action == GameLoopAction::LEADERBOARD ||
            update.gameloop_action == GameLoopAction::GAME_FINISHED ||
            update.gameloop_action == GameLoopAction::RETURN_TO_LOBBY) {
            race_ended = true;
            break;
        }
        if (update.gameloop_action == GameLoopAction::SEND_UPDATE_POSITIONS) {
            const auto& snapshot = std::get<WorldSnapshotDTO>(update.data);
            if (!snapshot.power_ups.empty() && !snapshot.power_ups[0].active) {
                collected = snapshot;
                break;
            }
        }
        update = client->protocol.receive_update();
    }

    // Back to the lobby before checking, so the client can leave cleanly even if the test fails
    if (race_ended) {
        if (update.gameloop_action == GameLoopAction::RETURN_TO_LOBBY)
            client->protocol.send_client_action(ClientAction::DISCONNECT);
        else
            client->return_to_lobby();
    } else {
        client->protocol.send_client_action(ClientAction::DISCONNECT);
        client->receive_until(GameLoopAction::RETURN_TO_LOBBY);
    }

    ASSERT_TRUE(collected.has_value()) << "the client never got the power-up inactive";
    ASSERT_EQ(collected->effects.size(), 1u);
    EXPECT_EQ(collected->effects[0].player_id, player_id);
    EXPECT_EQ(collected->effects[0].type, PowerUpType::Nitro);
    EXPECT_GT(collected->effects[0].remaining_ms, 0u);
}
