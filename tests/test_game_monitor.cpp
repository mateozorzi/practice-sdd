#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "../server/game_command.h"
#include "../server/game_monitor.h"

#include "server_test_utils.h"

namespace {

class GameMonitorTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    GameMonitor monitor{config};
};

GameUpdateDTO pop(Queue<GameUpdateDTO>& queue) {
    GameUpdateDTO update;
    EXPECT_TRUE(queue.try_pop(update));
    return update;
}

}  // namespace


TEST_F(GameMonitorTest, SendsUpdatesOnlyToTheTargetPlayer) {
    Queue<GameUpdateDTO>& first = monitor.register_player(1);
    Queue<GameUpdateDTO>& second = monitor.register_player(2);

    monitor.send_update_to_player(GameUpdateDTO(GameLoopAction::RACE_TIME, uint32_t{5}), 1);

    GameUpdateDTO update = pop(first);
    EXPECT_EQ(update.gameloop_action, GameLoopAction::RACE_TIME);
    EXPECT_EQ(std::get<uint32_t>(update.data), 5u);
    GameUpdateDTO none;
    EXPECT_FALSE(second.try_pop(none));
}

TEST_F(GameMonitorTest, UpdatesToUnknownPlayersAreDropped) {
    Queue<GameUpdateDTO>& queue = monitor.register_player(1);

    monitor.send_update_to_player(GameUpdateDTO(GameLoopAction::START_RACE), 99);

    GameUpdateDTO none;
    EXPECT_FALSE(queue.try_pop(none));
}

TEST_F(GameMonitorTest, BroadcastReachesEveryPlayer) {
    Queue<GameUpdateDTO>& first = monitor.register_player(1);
    Queue<GameUpdateDTO>& second = monitor.register_player(2);

    monitor.broadcast(GameUpdateDTO(GameLoopAction::START_COUNTDOWN));

    EXPECT_EQ(pop(first).gameloop_action, GameLoopAction::START_COUNTDOWN);
    EXPECT_EQ(pop(second).gameloop_action, GameLoopAction::START_COUNTDOWN);
}

TEST_F(GameMonitorTest, BroadcastDropsUpdatesWhenTheQueueIsFull) {
    config.server.snapshot_queue_capacity = 1;
    GameMonitor small_monitor(config);
    Queue<GameUpdateDTO>& queue = small_monitor.register_player(1);

    small_monitor.broadcast(GameUpdateDTO(GameLoopAction::START_SHOP));
    small_monitor.broadcast(GameUpdateDTO(GameLoopAction::START_RACE));

    EXPECT_EQ(pop(queue).gameloop_action, GameLoopAction::START_SHOP);
    GameUpdateDTO none;
    EXPECT_FALSE(queue.try_pop(none));
}

TEST_F(GameMonitorTest, UnregisterClosesThePlayerQueue) {
    Queue<GameUpdateDTO>& first = monitor.register_player(1);
    Queue<GameUpdateDTO>& second = monitor.register_player(2);

    monitor.unregister_player(1);
    // Unregistering twice or an unknown player is harmless
    monitor.unregister_player(1);
    monitor.unregister_player(99);

    GameUpdateDTO update;
    EXPECT_THROW(first.try_pop(update), ClosedQueue);
    EXPECT_FALSE(second.try_pop(update));

    // Broadcasting skips the closed queue
    monitor.broadcast(GameUpdateDTO(GameLoopAction::START_RACE));
    EXPECT_EQ(pop(second).gameloop_action, GameLoopAction::START_RACE);
}

TEST_F(GameMonitorTest, UnregisterAllClosesEveryQueue) {
    Queue<GameUpdateDTO>& first = monitor.register_player(1);
    Queue<GameUpdateDTO>& second = monitor.register_player(2);

    monitor.unregister_all_players();
    monitor.unregister_all_players();

    GameUpdateDTO update;
    EXPECT_THROW(first.try_pop(update), ClosedQueue);
    EXPECT_THROW(second.try_pop(update), ClosedQueue);
}

TEST_F(GameMonitorTest, RemovedPlayersStopReceivingUpdates) {
    monitor.register_player(1);
    Queue<GameUpdateDTO>& second = monitor.register_player(2);

    monitor.remove_player(1);
    monitor.remove_player(99);
    monitor.send_update_to_player(GameUpdateDTO(GameLoopAction::START_RACE), 1);
    monitor.broadcast(GameUpdateDTO(GameLoopAction::START_SHOP));

    EXPECT_EQ(pop(second).gameloop_action, GameLoopAction::START_SHOP);
}


TEST(GameCommandTest, DefaultCommandIsADisconnection) {
    GameCommand command;

    EXPECT_EQ(command.get_player_id(), 0u);
    EXPECT_EQ(command.get_client_action(), ClientAction::DISCONNECT);
    EXPECT_FALSE(command.get_data<CarDTO>().has_value());
}

TEST(GameCommandTest, KeepsThePlayerActionAndData) {
    GameCommand command(4, GameCommandDTO(ClientAction::SEND_CAR_SELECTION, CarDTO("Jeep", 100)));

    EXPECT_EQ(command.get_player_id(), 4u);
    EXPECT_EQ(command.get_client_action(), ClientAction::SEND_CAR_SELECTION);
    auto car = command.get_data<CarDTO>();
    ASSERT_TRUE(car.has_value());
    EXPECT_EQ(car->get().model, "Jeep");
    EXPECT_EQ(car->get().health, 100);
    EXPECT_FALSE(command.get_data<MovementDTO>().has_value());
}

TEST(GameCommandTest, KeepsMovementData) {
    GameCommand command(
            2, GameCommandDTO(ClientAction::SEND_CAR_MOVEMENT, MovementDTO(Direction::LEFT, true)));

    auto movement = command.get_data<MovementDTO>();
    ASSERT_TRUE(movement.has_value());
    EXPECT_EQ(movement->get().direction, Direction::LEFT);
    EXPECT_TRUE(movement->get().isMoving);
}
