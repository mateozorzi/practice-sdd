#include <algorithm>
#include <chrono>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "../server/game.h"
#include "../server/game_loop.h"

#include "server_test_utils.h"

// Integration tests: the game loop runs in its own thread and is driven only through the
// command queue, as the receivers do, while the tests read the updates the players get.

namespace {

void send(Queue<GameCommand>& commands, uint32_t player_id, ClientAction action,
          const std::variant<std::monostate, MovementDTO, PlayerDTO, CarDTO, std::string,
                             PlayerRaceStatsDTO, BuyUpgradeDTO>& data = std::monostate()) {
    commands.push(GameCommand(player_id, GameCommandDTO(action, data)));
}

void select_car(Queue<GameCommand>& commands, uint32_t player_id,
                const std::string& model = "Compact") {
    send(commands, player_id, ClientAction::SEND_CAR_SELECTION, CarDTO(model));
}

LeaderboardDTO wait_for_leaderboard(Queue<GameUpdateDTO>& queue) {
    auto update = wait_for_action(queue, GameLoopAction::SEND_LEADERBOARD);
    EXPECT_TRUE(update.has_value());
    if (!update)
        return LeaderboardDTO();
    return std::get<LeaderboardDTO>(update->data);
}

const PlayerRaceStatsDTO* find_stats(const LeaderboardDTO& leaderboard, uint32_t player_id) {
    auto it = std::find_if(
            leaderboard.players.begin(), leaderboard.players.end(),
            [player_id](const PlayerRaceStatsDTO& stats) { return stats.client_id == player_id; });
    return it == leaderboard.players.end() ? nullptr : &(*it);
}

// GameLoop used directly so every player is added before the loop thread starts
class GameLoopTest: public ::testing::Test {
protected:
    ServerConfig config = make_fast_game_config();
    RaceCatalogueDTO races{{make_test_race(3, 1, "first"), make_test_race(3, 1, "second")}};
    Queue<GameCommand> commands{1000};
    GameMonitor monitor{config};
    std::unique_ptr<GameLoop> loop;
    bool running = false;

    void create_loop(uint8_t max_players) {
        loop = std::make_unique<GameLoop>(monitor, commands, max_players, config, races);
    }

    Queue<GameUpdateDTO>& add_player(uint32_t player_id) {
        Queue<GameUpdateDTO>& queue = monitor.register_player(player_id);
        loop->add_player(player_id, "player" + std::to_string(player_id));
        return queue;
    }

    void start_loop() {
        loop->start();
        running = true;
    }

    void stop_loop() {
        loop->stop();
        loop->join();
        running = false;
    }

    void TearDown() override {
        if (running)
            stop_loop();
    }
};

class GameTest: public ::testing::Test {
protected:
    ServerConfig config = make_fast_game_config();
    RaceCatalogueDTO races{{make_test_race(3, 1, "only")}};
};

}  // namespace


TEST_F(GameLoopTest, LoopStateBeforeStarting) {
    create_loop(2);

    EXPECT_TRUE(loop->is_empty());
    EXPECT_EQ(loop->get_next_player_id(), 0u);

    add_player(5);
    add_player(5);

    EXPECT_FALSE(loop->is_empty());
    EXPECT_FALSE(loop->is_full());
    EXPECT_EQ(loop->get_active_players(), 1);
    EXPECT_TRUE(loop->has_player(5));
    EXPECT_FALSE(loop->has_player(6));
    EXPECT_EQ(loop->get_next_player_id(), 5u);
    EXPECT_FALSE(loop->has_started());

    add_player(6);
    EXPECT_TRUE(loop->is_full());

    loop->remove_player(5);
    loop->remove_player(42);
    EXPECT_EQ(loop->get_active_players(), 1);
    EXPECT_EQ(loop->get_next_player_id(), 6u);
}

TEST_F(GameLoopTest, WaitsUntilEveryPlayerSelectsACar) {
    create_loop(2);
    Queue<GameUpdateDTO>& first = add_player(1);
    add_player(2);
    start_loop();

    select_car(commands, 1);
    // Commands not allowed in this phase are discarded
    send(commands, 1, ClientAction::CHEAT_WIN_GAME);

    EXPECT_FALSE(wait_for_action(first, GameLoopAction::START_SHOP, std::chrono::milliseconds(200))
                         .has_value());
    EXPECT_FALSE(loop->has_started());

    select_car(commands, 2, "Truck");

    auto shop = wait_for_action(first, GameLoopAction::START_SHOP);
    ASSERT_TRUE(shop.has_value());
    EXPECT_EQ(std::get<uint8_t>(shop->data), config.game.penalty_per_upgrade);
    EXPECT_TRUE(loop->has_started());
}

TEST_F(GameLoopTest, PlaysEveryPhaseOfARace) {
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();

    select_car(commands, 1, "Sport");

    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::START_SHOP).has_value());

    auto race = wait_for_action(queue, GameLoopAction::SEND_RACE);
    ASSERT_TRUE(race.has_value());
    EXPECT_EQ(std::get<RaceDTO>(race->data).name, "first");

    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::START_COUNTDOWN).has_value());

    auto positions = wait_for_action(queue, GameLoopAction::SEND_UPDATE_POSITIONS);
    ASSERT_TRUE(positions.has_value());
    const WorldSnapshotDTO& snapshot = std::get<WorldSnapshotDTO>(positions->data);
    ASSERT_EQ(snapshot.car_positions.size(), 1u);
    EXPECT_EQ(snapshot.car_positions[0].first.id, 1u);
    EXPECT_EQ(snapshot.car_positions[0].first.car.model, "Sport");
    EXPECT_EQ(snapshot.car_positions[0].second.next.id, 1);

    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::START_RACE).has_value());
    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::RACE_TIME).has_value());
}

TEST_F(GameLoopTest, CarsMoveDuringTheRace) {
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();
    select_car(commands, 1);
    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::START_RACE).has_value());

    auto before = wait_for_action(queue, GameLoopAction::SEND_UPDATE_POSITIONS);
    ASSERT_TRUE(before.has_value());
    const CarPositionDTO start = std::get<WorldSnapshotDTO>(before->data).car_positions[0].second;

    send(commands, 1, ClientAction::SEND_CAR_MOVEMENT, MovementDTO(Direction::UP, true));

    auto moved = wait_for_update(queue, [&start](const GameUpdateDTO& update) {
        if (update.gameloop_action != GameLoopAction::SEND_UPDATE_POSITIONS)
            return false;
        const auto& car = std::get<WorldSnapshotDTO>(update.data).car_positions[0].second;
        return car.x_position != start.x_position || car.y_position != start.y_position;
    });
    EXPECT_TRUE(moved.has_value());
}

TEST_F(GameLoopTest, WinRaceCheatGoesToTheLeaderboard) {
    create_loop(2);
    Queue<GameUpdateDTO>& first = add_player(1);
    Queue<GameUpdateDTO>& second = add_player(2);
    start_loop();
    select_car(commands, 1);
    select_car(commands, 2);
    ASSERT_TRUE(wait_for_action(first, GameLoopAction::START_RACE).has_value());

    send(commands, 2, ClientAction::CHEAT_WIN_RACE);

    ASSERT_TRUE(wait_for_action(first, GameLoopAction::LEADERBOARD).has_value());
    LeaderboardDTO leaderboard = wait_for_leaderboard(first);
    EXPECT_EQ(leaderboard.race_name, "first");
    ASSERT_EQ(leaderboard.players.size(), 2u);

    const PlayerRaceStatsDTO* winner = find_stats(leaderboard, 2);
    const PlayerRaceStatsDTO* loser = find_stats(leaderboard, 1);
    ASSERT_NE(winner, nullptr);
    ASSERT_NE(loser, nullptr);
    EXPECT_EQ(winner->position, 1);
    EXPECT_EQ(winner->points, 8);
    EXPECT_EQ(loser->position, 2);
    EXPECT_EQ(loser->points, 0);
    EXPECT_EQ(loser->total_time, config.game.timeout_seconds_race);

    // After the leaderboard the next race starts
    ASSERT_TRUE(wait_for_action(second, GameLoopAction::LEADERBOARD).has_value());
    auto next_race = wait_for_action(second, GameLoopAction::SEND_RACE);
    ASSERT_TRUE(next_race.has_value());
    EXPECT_EQ(std::get<RaceDTO>(next_race->data).name, "second");
}

TEST_F(GameLoopTest, FinishingEveryRaceEndsTheGame) {
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();
    select_car(commands, 1);

    for (int race = 0; race < 2; ++race) {
        ASSERT_TRUE(wait_for_action(queue, GameLoopAction::START_RACE).has_value());
        send(commands, 1, ClientAction::CHEAT_WIN_RACE);
        ASSERT_TRUE(wait_for_action(queue, GameLoopAction::LEADERBOARD).has_value());
    }

    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::GAME_FINISHED).has_value());
    LeaderboardDTO final_leaderboard = wait_for_leaderboard(queue);
    EXPECT_EQ(final_leaderboard.race_name, "Final Leaderboard");
    ASSERT_EQ(final_leaderboard.players.size(), 1u);
    EXPECT_EQ(final_leaderboard.players[0].position, 1);
    EXPECT_EQ(final_leaderboard.players[0].points, 16);

    // The game finished phase lasts zero seconds, then the loop sends everyone to the lobby
    EXPECT_TRUE(wait_for_action(queue, GameLoopAction::RETURN_TO_LOBBY).has_value());

    // and empties itself, so the lobby can reap it
    stop_loop();
    EXPECT_TRUE(loop->is_empty());
}

TEST_F(GameLoopTest, WinGameCheatSkipsTheRemainingRaces) {
    create_loop(2);
    Queue<GameUpdateDTO>& first = add_player(1);
    add_player(2);
    start_loop();
    select_car(commands, 1);
    select_car(commands, 2);
    ASSERT_TRUE(wait_for_action(first, GameLoopAction::START_RACE).has_value());

    send(commands, 1, ClientAction::CHEAT_WIN_GAME);

    auto finished = wait_for_update(first, [](const GameUpdateDTO& update) {
        return update.gameloop_action == GameLoopAction::GAME_FINISHED ||
               update.gameloop_action == GameLoopAction::LEADERBOARD;
    });
    ASSERT_TRUE(finished.has_value());
    EXPECT_EQ(finished->gameloop_action, GameLoopAction::GAME_FINISHED);

    LeaderboardDTO final_leaderboard = wait_for_leaderboard(first);
    ASSERT_EQ(final_leaderboard.players.size(), 2u);
    EXPECT_EQ(final_leaderboard.players[0].client_id, 1u);
    EXPECT_EQ(final_leaderboard.players[0].position, 1);
}

TEST_F(GameLoopTest, UpgradeShopAcceptsUpgrades) {
    config.game.wait_seconds_upgrade_shop = 1;
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();
    select_car(commands, 1);
    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::START_SHOP).has_value());

    auto remaining = wait_for_action(queue, GameLoopAction::SEND_UPGRADES_REMAINING);
    ASSERT_TRUE(remaining.has_value());
    EXPECT_EQ(std::get<uint8_t>(remaining->data), MAX_UPGRADES_PER_RACE);

    send(commands, 1, ClientAction::SEND_UPGRADE, BuyUpgradeDTO{ButtonType::Health});

    auto after_upgrade = wait_for_update(queue, [](const GameUpdateDTO& update) {
        return update.gameloop_action == GameLoopAction::SEND_UPGRADES_REMAINING &&
               std::get<uint8_t>(update.data) == MAX_UPGRADES_PER_RACE - 1;
    });
    EXPECT_TRUE(after_upgrade.has_value());

    auto shop_time = wait_for_action(queue, GameLoopAction::SEND_UPGRADE_SHOP_TIME_REMAINING);
    ASSERT_TRUE(shop_time.has_value());
    EXPECT_LE(std::get<uint8_t>(shop_time->data), 1);

    auto positions = wait_for_action(queue, GameLoopAction::SEND_UPDATE_POSITIONS);
    ASSERT_TRUE(positions.has_value());
    const PlayerDTO& player = std::get<WorldSnapshotDTO>(positions->data).car_positions[0].first;
    EXPECT_EQ(player.max_health,
              config.physics.compact.health +
                      static_cast<uint16_t>(config.game.upgrades.health_upgrade));
}

TEST_F(GameLoopTest, CountdownSendsTheRemainingTime) {
    config.game.wait_seconds_countdown = 1;
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();
    select_car(commands, 1);

    auto countdown = wait_for_action(queue, GameLoopAction::COUNTDOWN_TIME);
    ASSERT_TRUE(countdown.has_value());
    EXPECT_LE(std::get<uint8_t>(countdown->data), 1);
}

TEST_F(GameLoopTest, InfiniteHealthCheatIsAccepted) {
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();
    select_car(commands, 1);
    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::START_RACE).has_value());

    send(commands, 1, ClientAction::CHEAT_INFINITE_HEALTH);
    send(commands, 99, ClientAction::CHEAT_INFINITE_HEALTH);

    auto positions = wait_for_action(queue, GameLoopAction::SEND_UPDATE_POSITIONS);
    ASSERT_TRUE(positions.has_value());
    const PlayerDTO& player = std::get<WorldSnapshotDTO>(positions->data).car_positions[0].first;
    EXPECT_FALSE(player.dead);
    EXPECT_EQ(player.health, config.physics.compact.health);
}

TEST_F(GameLoopTest, RaceTimeoutFinishesTheRace) {
    config.game.timeout_seconds_race = 0;
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();
    select_car(commands, 1);

    ASSERT_TRUE(wait_for_action(queue, GameLoopAction::LEADERBOARD).has_value());
    LeaderboardDTO leaderboard = wait_for_leaderboard(queue);
    ASSERT_EQ(leaderboard.players.size(), 1u);
    EXPECT_EQ(leaderboard.players[0].points, 0);
}

TEST_F(GameLoopTest, DisconnectionSendsThePlayerBackToTheLobby) {
    create_loop(2);
    Queue<GameUpdateDTO>& first = add_player(1);
    Queue<GameUpdateDTO>& second = add_player(2);
    start_loop();

    send(commands, 1, ClientAction::DISCONNECT);

    ASSERT_TRUE(wait_for_action(first, GameLoopAction::RETURN_TO_LOBBY).has_value());
    // After the update the queue is closed
    EXPECT_FALSE(wait_for_action(first, GameLoopAction::START_SHOP, std::chrono::milliseconds(100))
                         .has_value());

    GameUpdateDTO update;
    EXPECT_FALSE(second.try_pop(update));
}

TEST_F(GameLoopTest, StopSendsEveryPlayerToTheLobby) {
    create_loop(1);
    Queue<GameUpdateDTO>& queue = add_player(1);
    start_loop();

    stop_loop();

    EXPECT_TRUE(wait_for_action(queue, GameLoopAction::RETURN_TO_LOBBY).has_value());
}


TEST_F(GameTest, ExposesTheGameData) {
    Game game(2, config, "my game", races);

    EXPECT_EQ(game.get_name(), "my game");
    EXPECT_EQ(game.get_max_players(), 2);
    EXPECT_TRUE(game.is_empty());
    EXPECT_FALSE(game.is_full());
    EXPECT_FALSE(game.has_started());
}

TEST_F(GameTest, AddingPlayersStartsTheLoop) {
    Game game(1, config, "my game", races);

    auto [commands, updates] = game.add_player(3, "mateo");

    EXPECT_TRUE(game.is_full());
    EXPECT_TRUE(game.has_player(3));
    EXPECT_EQ(game.get_active_players(), 1);
    EXPECT_EQ(game.get_next_player_id(), 3u);

    select_car(commands, 3);
    ASSERT_TRUE(wait_for_action(updates, GameLoopAction::START_SHOP).has_value());
    EXPECT_TRUE(game.has_started());
}

TEST_F(GameTest, StopClosesTheQueues) {
    Game game(2, config, "my game", races);
    auto [commands, updates] = game.add_player(3, "mateo");

    ASSERT_TRUE(game.has_player(3));
    (void)updates;  // destroyed by stop

    game.stop();

    EXPECT_THROW(commands.push(GameCommand()), ClosedQueue);
}
