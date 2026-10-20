#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include <gtest/gtest.h>
#include <sys/socket.h>

#include "../../client/client_protocol.h"
#include "../../client/receiver.h"
#include "../../client/sender.h"
#include "../../common/socket.h"
#include "../../server/server_protocol.h"

// Integration tests of the client network threads: they talk through real localhost sockets
// to a peer that uses the server protocol.

namespace {

std::atomic<int> next_port{26500};

std::optional<GameUpdateDTO> pop_update(
        Queue<GameUpdateDTO>& queue,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(2000)) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    GameUpdateDTO update;
    while (std::chrono::steady_clock::now() < deadline) {
        if (queue.try_pop(update))
            return update;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return std::nullopt;
}

bool wait_until_dead(const Thread& thread) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
    while (thread.is_alive() && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return !thread.is_alive();
}

class ClientThreadsTest: public ::testing::Test {
protected:
    std::string port = std::to_string(next_port++);
    Socket acceptor{port.c_str()};
    Socket client_socket{"localhost", port.c_str()};
    Socket server_socket = acceptor.accept();
    ClientProtocol client{client_socket};
    ServerProtocol server{server_socket};
};

class ReceiverTest: public ClientThreadsTest {
protected:
    Receiver receiver{client};

    void SetUp() override { receiver.start(); }

    void TearDown() override {
        receiver.stop();
        // Unblock the receive if the test left it waiting
        client_socket.shutdown(SHUT_RDWR);
        receiver.join();
    }
};

class SenderTest: public ClientThreadsTest {
protected:
    Sender sender{client};

    void SetUp() override { sender.start(); }

    void TearDown() override {
        sender.stop();
        sender.close_queue();
        sender.join();
    }
};

}  // namespace


TEST_F(SenderTest, SendsMovements) {
    sender.add_event(MovementDTO(Direction::LEFT, true));

    GameCommandDTO command = server.receive_game_command();

    EXPECT_EQ(command.client_action, ClientAction::SEND_CAR_MOVEMENT);
    auto movement = command.get_data<MovementDTO>();
    ASSERT_TRUE(movement.has_value());
    EXPECT_EQ(movement->get().direction, Direction::LEFT);
    EXPECT_TRUE(movement->get().isMoving);
}

TEST_F(SenderTest, SendsUpgradePurchases) {
    sender.add_event(BuyUpgradeDTO{ButtonType::Handling});

    GameCommandDTO command = server.receive_game_command();

    EXPECT_EQ(command.client_action, ClientAction::SEND_UPGRADE);
    auto upgrade = command.get_data<BuyUpgradeDTO>();
    ASSERT_TRUE(upgrade.has_value());
    EXPECT_EQ(upgrade->get().upgrade, ButtonType::Handling);
}

TEST_F(SenderTest, SendsActionsInOrder) {
    sender.add_event(ClientActionDTO(ClientAction::CHEAT_WIN_RACE));
    sender.add_event(ClientActionDTO(ClientAction::CHEAT_INFINITE_HEALTH));
    sender.add_event(ClientActionDTO(ClientAction::DISCONNECT));

    EXPECT_EQ(server.receive_game_command().client_action, ClientAction::CHEAT_WIN_RACE);
    EXPECT_EQ(server.receive_game_command().client_action, ClientAction::CHEAT_INFINITE_HEALTH);
    EXPECT_EQ(server.receive_game_command().client_action, ClientAction::DISCONNECT);
}

TEST_F(SenderTest, ClosingTheQueueEndsTheThread) {
    sender.close_queue();
    sender.close_queue();

    EXPECT_TRUE(wait_until_dead(sender));
}

// Disabled: client bug. Socket::sendall returns 0 on a broken pipe instead of throwing and
// CommonProtocol::send_buffer ignores it, so the sender never notices the server is gone.
TEST_F(SenderTest, DISABLED_SendingToAClosedConnectionEndsTheThread) {
    server_socket.shutdown(SHUT_RDWR);
    server_socket.close();

    // The OS may buffer the first sends: keep adding events until the sender gives up
    for (int i = 0; i < 1000 && sender.is_alive(); ++i) {
        sender.add_event(MovementDTO(Direction::UP, true));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_TRUE(wait_until_dead(sender));
}


TEST_F(ReceiverTest, QueuesTheUpdatesItReceives) {
    server.send_game_update(GameUpdateDTO(GameLoopAction::START_SHOP, uint8_t{10}));
    server.send_game_update(GameUpdateDTO(GameLoopAction::RACE_TIME, uint32_t{42}));
    server.send_game_update(GameUpdateDTO(GameLoopAction::START_RACE));

    Queue<GameUpdateDTO>& queue = receiver.get_car_positions_queue();

    auto shop = pop_update(queue);
    ASSERT_TRUE(shop.has_value());
    EXPECT_EQ(shop->gameloop_action, GameLoopAction::START_SHOP);
    EXPECT_EQ(std::get<uint8_t>(shop->data), 10);

    auto time = pop_update(queue);
    ASSERT_TRUE(time.has_value());
    EXPECT_EQ(std::get<uint32_t>(time->data), 42u);

    auto start = pop_update(queue);
    ASSERT_TRUE(start.has_value());
    EXPECT_EQ(start->gameloop_action, GameLoopAction::START_RACE);
    EXPECT_TRUE(receiver.is_alive());
}

TEST_F(ReceiverTest, ReceivesRacesSnapshotsAndLeaderboards) {
    RaceDTO race("race", "Vice City", {CheckpointDTO(1, 'H', 10, 20, 66, 15)}, 3);
    WorldSnapshotDTO snapshot;
    snapshot.add_car_position(PlayerDTO("mateo", 1, CarDTO("Jeep"), 90, false, 100),
                              CarPositionDTO(1, 100, 200, 90));
    snapshot.add_npc_position(NpcDTO(4, 5, 6, 7));
    LeaderboardDTO leaderboard("race", {PlayerRaceStatsDTO(1, "mateo", 1, 8, 30)});

    server.send_game_update(GameUpdateDTO(GameLoopAction::SEND_RACE, race));
    server.send_game_update(GameUpdateDTO(GameLoopAction::SEND_UPDATE_POSITIONS, snapshot));
    server.send_game_update(GameUpdateDTO(GameLoopAction::SEND_LEADERBOARD, leaderboard));

    Queue<GameUpdateDTO>& queue = receiver.get_car_positions_queue();

    auto race_update = pop_update(queue);
    ASSERT_TRUE(race_update.has_value());
    const RaceDTO& received_race = std::get<RaceDTO>(race_update->data);
    EXPECT_EQ(received_race.city, "Vice City");
    EXPECT_EQ(received_race.laps, 3);
    ASSERT_EQ(received_race.checkpoints.size(), 1u);

    auto positions = pop_update(queue);
    ASSERT_TRUE(positions.has_value());
    const WorldSnapshotDTO& received = std::get<WorldSnapshotDTO>(positions->data);
    ASSERT_EQ(received.car_positions.size(), 1u);
    EXPECT_EQ(received.car_positions[0].first.username, "mateo");
    EXPECT_EQ(received.car_positions[0].second.x_position, 100u);
    ASSERT_EQ(received.npc_positions.size(), 1u);
    EXPECT_EQ(received.npc_positions[0].npc_id, 4u);

    auto board = pop_update(queue);
    ASSERT_TRUE(board.has_value());
    const LeaderboardDTO& received_board = std::get<LeaderboardDTO>(board->data);
    ASSERT_EQ(received_board.players.size(), 1u);
    EXPECT_EQ(received_board.players[0].points, 8);
}

TEST_F(ReceiverTest, ReturnToLobbyEndsTheThread) {
    server.send_game_update(GameUpdateDTO(GameLoopAction::RETURN_TO_LOBBY));

    auto update = pop_update(receiver.get_car_positions_queue());
    ASSERT_TRUE(update.has_value());
    EXPECT_EQ(update->gameloop_action, GameLoopAction::RETURN_TO_LOBBY);
    EXPECT_TRUE(wait_until_dead(receiver));
    EXPECT_TRUE(receiver.is_server_connected());
}

TEST_F(ReceiverTest, ServerDisconnectionEndsTheThread) {
    server.send_disconnection();

    auto update = pop_update(receiver.get_car_positions_queue());
    ASSERT_TRUE(update.has_value());
    EXPECT_EQ(update->gameloop_action, GameLoopAction::DISCONNECT);
    EXPECT_TRUE(wait_until_dead(receiver));
    EXPECT_FALSE(receiver.is_server_connected());
}

// Disabled: client bug. Socket::recvall returns 0 when the peer closes and CommonProtocol reads
// it as a zero value, so the receiver loops forever queueing updates with action 0.
TEST_F(ReceiverTest, DISABLED_LostConnectionEndsTheThread) {
    server_socket.shutdown(SHUT_RDWR);
    server_socket.close();

    EXPECT_TRUE(wait_until_dead(receiver));
}
