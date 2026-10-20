#include <string>

#include <gtest/gtest.h>

#include "../server/lobby.h"

#include "server_test_utils.h"

namespace {

class LobbyTest: public ::testing::Test {
protected:
    ServerConfig config = make_fast_game_config();
    RaceCatalogueDTO races{{make_test_race(3, 1)}};
    Lobby lobby{config};

    std::optional<GameQueues> create_game(uint32_t host_id, uint8_t max_players = 2,
                                          const std::string& name = "game") {
        return lobby.create_new_game(host_id, "host", max_players, name, races);
    }
};

}  // namespace


TEST_F(LobbyTest, ListsTheRacesInTheRepository) {
    RaceCatalogueDTO catalogue = lobby.list_races();

    EXPECT_EQ(catalogue.size, 6);
    EXPECT_EQ(catalogue.races.size(), 6u);
}

TEST_F(LobbyTest, NoGamesAtStart) {
    EXPECT_EQ(lobby.list_games().size, 0);
    EXPECT_EQ(lobby.get_game_id(1), 0u);
}

TEST_F(LobbyTest, CreatedGamesAreListedWithTheHostIdAsGameId) {
    ASSERT_TRUE(create_game(10, 3, "my game").has_value());

    LobbyCatalogueDTO games = lobby.list_games();

    ASSERT_EQ(games.lobbies.size(), 1u);
    EXPECT_EQ(games.lobbies[0].lobby_id, 10u);
    EXPECT_EQ(games.lobbies[0].lobby_name, "my game");
    EXPECT_EQ(games.lobbies[0].max_players, 3);
    EXPECT_EQ(games.lobbies[0].active_players, 1);
    EXPECT_EQ(lobby.get_game_id(10), 10u);
}

TEST_F(LobbyTest, PlayersCanJoinUntilTheGameIsFull) {
    ASSERT_TRUE(create_game(10, 2).has_value());

    EXPECT_TRUE(lobby.join_existing_game(11, 10, "guest").has_value());
    EXPECT_FALSE(lobby.join_existing_game(12, 10, "late").has_value());

    EXPECT_EQ(lobby.get_game_id(11), 10u);
    EXPECT_EQ(lobby.get_game_id(12), 0u);
    EXPECT_EQ(lobby.list_games().lobbies[0].active_players, 2);
}

TEST_F(LobbyTest, JoiningAnUnknownGameFails) {
    EXPECT_FALSE(lobby.join_existing_game(11, 999, "guest").has_value());
}

TEST_F(LobbyTest, ConcurrentGamesAreLimited) {
    for (uint32_t host = 1; host <= config.server.max_concurrent_games; ++host)
        ASSERT_TRUE(create_game(host).has_value());

    EXPECT_FALSE(create_game(100).has_value());
    EXPECT_EQ(lobby.list_games().size, config.server.max_concurrent_games);
}

TEST_F(LobbyTest, StartedGamesAreNotListed) {
    auto queues = create_game(10, 1);
    ASSERT_TRUE(queues.has_value());

    queues->first.push(
            GameCommand(10, GameCommandDTO(ClientAction::SEND_CAR_SELECTION, CarDTO("Compact"))));
    ASSERT_TRUE(wait_for_action(queues->second, GameLoopAction::START_SHOP).has_value());

    EXPECT_EQ(lobby.list_games().size, 0);
    EXPECT_EQ(lobby.get_game_id(10), 10u);
}

TEST_F(LobbyTest, HostLeavingHandsTheGameToAnotherPlayer) {
    ASSERT_TRUE(create_game(10, 3).has_value());
    ASSERT_TRUE(lobby.join_existing_game(11, 10, "guest").has_value());

    lobby.remove_player(10, 10);

    EXPECT_EQ(lobby.get_game_id(10), 0u);
    EXPECT_EQ(lobby.get_game_id(11), 11u);
    ASSERT_EQ(lobby.list_games().lobbies.size(), 1u);
    EXPECT_EQ(lobby.list_games().lobbies[0].lobby_id, 11u);
}

TEST_F(LobbyTest, GuestLeavingKeepsTheGameId) {
    ASSERT_TRUE(create_game(10, 3).has_value());
    ASSERT_TRUE(lobby.join_existing_game(11, 10, "guest").has_value());

    lobby.remove_player(11, 10);
    lobby.remove_player(11, 999);

    EXPECT_EQ(lobby.get_game_id(10), 10u);
    EXPECT_EQ(lobby.list_games().lobbies[0].active_players, 1);
}

TEST_F(LobbyTest, ReapRemovesOnlyEmptyGames) {
    ASSERT_TRUE(create_game(10).has_value());
    ASSERT_TRUE(create_game(20).has_value());

    lobby.remove_player(10, 10);
    lobby.reap();

    LobbyCatalogueDTO games = lobby.list_games();
    ASSERT_EQ(games.lobbies.size(), 1u);
    EXPECT_EQ(games.lobbies[0].lobby_id, 20u);
}

TEST_F(LobbyTest, StopRemovesEveryGame) {
    ASSERT_TRUE(create_game(10).has_value());
    ASSERT_TRUE(create_game(20).has_value());

    lobby.stop();

    EXPECT_EQ(lobby.list_games().size, 0);
    EXPECT_TRUE(create_game(30).has_value());
}
