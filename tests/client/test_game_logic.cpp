#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../../client/game_logic.h"

// Unit tests of the client game logic: the server updates are pushed into the queue the
// receiver would fill, and a fake view records what the logic asks the screen to do.

namespace {

constexpr uint32_t MY_ID = 1;
constexpr uint32_t OTHER_ID = 2;

class FakeView: public GameView {
public:
    std::vector<uint32_t> sound_ids;
    std::vector<std::string> loaded_cities;
    int shop_opened = 0;
    int shop_closed = 0;
    int leaderboard_opened = 0;
    int leaderboard_closed = 0;

    void set_sound_info(const std::vector<uint32_t>& ids) override { sound_ids = ids; }
    void load_city(const std::string& city_name, const std::string&, const std::string&) override {
        loaded_cities.push_back(city_name);
    }
    void open_shop() override { shop_opened++; }
    void close_shop() override { shop_closed++; }
    void open_leaderboard() override { leaderboard_opened++; }
    void close_leaderboard() override { leaderboard_closed++; }
};

std::pair<PlayerDTO, CarPositionDTO> car(uint32_t id, uint32_t x, uint32_t y, uint16_t health,
                                         bool dead = false) {
    return {PlayerDTO("player" + std::to_string(id), id, CarDTO("Jeep"), health, dead, 100),
            CarPositionDTO(id, x, y, 90, CheckpointDTO(3, 'H'), CarLayer::ON_BRIDGE, 2, 3)};
}

GameUpdateDTO snapshot(const std::vector<std::pair<PlayerDTO, CarPositionDTO>>& cars,
                       const std::vector<NpcDTO>& npcs = {}) {
    WorldSnapshotDTO world(cars);
    for (const auto& npc: npcs) world.add_npc_position(npc);
    return GameUpdateDTO(GameLoopAction::SEND_UPDATE_POSITIONS, world);
}

class GameLogicTest: public ::testing::Test {
protected:
    FakeView view;
    Queue<GameUpdateDTO> updates;
    GameLogic logic{view, updates, MY_ID};

    void push(const GameUpdateDTO& update) { updates.push(update); }

    void push(GameLoopAction action) { updates.push(GameUpdateDTO(action)); }

    LocalInfo& state() { return logic.get_state(); }

    // Takes the logic through the phases as the server would, up to the race
    void go_to_race() {
        push(GameUpdateDTO(GameLoopAction::START_SHOP, uint8_t{10}));
        ASSERT_FALSE(logic.update_waiting());
        push(GameLoopAction::START_COUNTDOWN);
        ASSERT_FALSE(logic.update_upgrade());
        push(GameLoopAction::START_RACE);
        ASSERT_FALSE(logic.update_countdown());
        ASSERT_EQ(state().phase, Phase::IN_RACE);
    }
};

}  // namespace


// Waiting for the other players

TEST_F(GameLogicTest, StartsWaitingForCarSelection) {
    EXPECT_TRUE(logic.is_running());
    EXPECT_EQ(state().id, MY_ID);
    EXPECT_EQ(state().phase, Phase::WAITING_FOR_CAR_SELECTION);
    EXPECT_TRUE(logic.update_waiting());
}

TEST_F(GameLogicTest, ShopStartMovesToTheUpgradePhase) {
    push(GameLoopAction::RACE_TIME);  // ignored while waiting
    push(GameUpdateDTO(GameLoopAction::START_SHOP, uint8_t{15}));

    EXPECT_FALSE(logic.update_waiting());

    EXPECT_EQ(state().phase, Phase::UPGRADE);
    EXPECT_EQ(state().penalty_per_upgrade, 15);
    EXPECT_EQ(view.sound_ids, std::vector<uint32_t>{MY_ID});
}

TEST_F(GameLogicTest, ReturnToLobbyStopsTheGameInEveryPhase) {
    using Update = bool (GameLogic::*)();
    for (Update update: {&GameLogic::update_waiting, &GameLogic::update_upgrade,
                         &GameLogic::update_countdown, &GameLogic::update_race,
                         &GameLogic::update_leaderboard, &GameLogic::update_game_finished}) {
        FakeView fresh_view;
        Queue<GameUpdateDTO> queue;
        GameLogic fresh(fresh_view, queue, MY_ID);
        queue.push(GameUpdateDTO(GameLoopAction::RETURN_TO_LOBBY));

        EXPECT_TRUE((fresh.*update)());
        EXPECT_FALSE(fresh.is_running());
    }
}

TEST_F(GameLogicTest, ServerDisconnectionStopsTheGame) {
    push(GameLoopAction::DISCONNECT);

    EXPECT_TRUE(logic.update_waiting());
    EXPECT_FALSE(logic.is_running());
}

TEST_F(GameLogicTest, StopEndsTheGame) {
    logic.stop();

    EXPECT_FALSE(logic.is_running());
}


// Upgrade shop

TEST_F(GameLogicTest, UpgradeShopIsOpenedOnEveryUpdate) {
    EXPECT_TRUE(logic.update_upgrade());
    EXPECT_TRUE(logic.update_upgrade());

    EXPECT_EQ(view.shop_opened, 2);
}

TEST_F(GameLogicTest, CityIsLoadedOnlyWhenItChanges) {
    push(GameUpdateDTO(GameLoopAction::SEND_RACE, RaceDTO("first", "Vice City", {}, 1)));
    push(GameUpdateDTO(GameLoopAction::SEND_RACE, RaceDTO("second", "Vice City", {}, 1)));
    push(GameUpdateDTO(GameLoopAction::SEND_RACE, RaceDTO("third", "San Andreas", {}, 1)));

    EXPECT_TRUE(logic.update_upgrade());

    EXPECT_EQ(view.loaded_cities, (std::vector<std::string>{"Vice City", "San Andreas"}));
}

TEST_F(GameLogicTest, ShopKeepsUpgradesAndTimeRemaining) {
    push(GameUpdateDTO(GameLoopAction::SEND_UPGRADES_REMAINING, uint8_t{3}));
    push(GameUpdateDTO(GameLoopAction::SEND_UPGRADE_SHOP_TIME_REMAINING, uint8_t{7}));

    EXPECT_TRUE(logic.update_upgrade());

    EXPECT_EQ(state().player.upgrades_remaining, 3);
    EXPECT_EQ(state().upgrade_shop_countdown, 7);
}

TEST_F(GameLogicTest, CountdownStartClosesTheShop) {
    push(GameLoopAction::START_COUNTDOWN);

    EXPECT_FALSE(logic.update_upgrade());

    EXPECT_EQ(state().phase, Phase::COUNTDOWN);
    EXPECT_EQ(view.shop_closed, 1);
}

TEST_F(GameLogicTest, ShopAppliesSnapshotsFollowedByAnotherUpdate) {
    push(snapshot({car(MY_ID, 100, 200, 80), car(OTHER_ID, 300, 400, 50)}));
    push(GameUpdateDTO(GameLoopAction::SEND_UPGRADES_REMAINING, uint8_t{4}));

    logic.update_upgrade();

    EXPECT_EQ(state().player.x, 100);
    EXPECT_EQ(state().player.life, 80);
    ASSERT_EQ(state().players.count(OTHER_ID), 1u);
    EXPECT_EQ(state().players[OTHER_ID].y, 400);
}


// Countdown

TEST_F(GameLogicTest, CountdownKeepsTheSecondsLeftAndStartsTheRace) {
    push(GameUpdateDTO(GameLoopAction::COUNTDOWN_TIME, uint8_t{2}));
    EXPECT_TRUE(logic.update_countdown());
    EXPECT_EQ(state().countdown_seconds, 2);

    push(snapshot({car(MY_ID, 10, 20, 100)}));
    push(GameLoopAction::START_RACE);
    EXPECT_FALSE(logic.update_countdown());

    EXPECT_EQ(state().phase, Phase::IN_RACE);
    EXPECT_EQ(state().player.x, 10);
}


// Race

TEST_F(GameLogicTest, RaceAppliesTheLastSnapshot) {
    go_to_race();
    push(snapshot({car(MY_ID, 100, 200, 80)}, {NpcDTO(9, 1, 2, 3)}));
    push(snapshot({car(MY_ID, 110, 210, 80)}, {NpcDTO(9, 4, 5, 6)}));

    EXPECT_TRUE(logic.update_race());

    const PlayerInfo& me = state().player;
    EXPECT_EQ(me.id, MY_ID);
    EXPECT_EQ(me.x, 110);
    EXPECT_EQ(me.y, 210);
    EXPECT_EQ(me.angle, 90);
    EXPECT_EQ(me.model, "Jeep");
    EXPECT_EQ(me.username, "player1");
    EXPECT_EQ(me.max_life, 100);
    EXPECT_EQ(me.layer, CarLayer::ON_BRIDGE);
    EXPECT_EQ(me.checkpoint.id, 3);
    EXPECT_EQ(me.current_lap, 2);
    EXPECT_EQ(me.race_lap, 3);
    ASSERT_EQ(state().npcs.size(), 1u);
    EXPECT_EQ(state().npcs[0].x_position, 4u);
}

TEST_F(GameLogicTest, RaceKeepsTheElapsedTime) {
    go_to_race();
    push(GameUpdateDTO(GameLoopAction::RACE_TIME, uint32_t{42}));

    EXPECT_TRUE(logic.update_race());

    EXPECT_EQ(state().player.time, 42);
}

TEST_F(GameLogicTest, LeaderboardEndsTheRaceAndResetsTheTime) {
    go_to_race();
    push(GameUpdateDTO(GameLoopAction::RACE_TIME, uint32_t{42}));
    push(snapshot({car(MY_ID, 5, 5, 100)}));
    push(GameLoopAction::LEADERBOARD);

    EXPECT_FALSE(logic.update_race());

    EXPECT_EQ(state().phase, Phase::LEADERBOARD);
    EXPECT_EQ(state().player.time, 0);
    EXPECT_EQ(state().player.x, 5);
}

TEST_F(GameLogicTest, GameFinishedEndsTheRace) {
    go_to_race();
    push(snapshot({car(MY_ID, 7, 7, 100)}));
    push(GameLoopAction::GAME_FINISHED);

    EXPECT_FALSE(logic.update_race());

    EXPECT_EQ(state().phase, Phase::GAME_FINISHED);
    EXPECT_EQ(state().player.x, 7);
}

TEST_F(GameLogicTest, RaceUpdatesOutsideTheRacePhaseChangePhase) {
    push(GameUpdateDTO(GameLoopAction::RACE_TIME, uint32_t{1}));

    EXPECT_FALSE(logic.update_race());
}


// Events detected between snapshots

TEST_F(GameLogicTest, DetectsMyMovementCrashesAndDeath) {
    go_to_race();
    push(snapshot({car(MY_ID, 100, 100, 100)}));
    logic.update_race();
    EXPECT_TRUE(state().player.isMoving);  // from (0, 0) to (100, 100)

    logic.update_waiting();  // resets the events
    EXPECT_FALSE(state().player.isMoving);

    push(snapshot({car(MY_ID, 100, 100, 60)}));
    logic.update_race();
    EXPECT_FALSE(state().player.isMoving);
    EXPECT_TRUE(state().player.crashed);
    EXPECT_FALSE(state().player.died);

    push(snapshot({car(MY_ID, 100, 100, 0, true)}));
    logic.update_race();
    EXPECT_TRUE(state().player.died);
}

TEST_F(GameLogicTest, DetectsOtherPlayersEventsFromTheirPreviousState) {
    go_to_race();
    push(snapshot({car(OTHER_ID, 100, 100, 100)}));
    logic.update_race();
    // First time seen: there is nothing to compare with
    EXPECT_FALSE(state().players[OTHER_ID].isMoving);

    push(snapshot({car(OTHER_ID, 150, 100, 70)}));
    logic.update_race();
    EXPECT_TRUE(state().players[OTHER_ID].isMoving);
    EXPECT_TRUE(state().players[OTHER_ID].crashed);

    logic.update_upgrade();  // resets the events
    EXPECT_FALSE(state().players[OTHER_ID].isMoving);
    EXPECT_FALSE(state().players[OTHER_ID].crashed);
}

TEST_F(GameLogicTest, PlayersMissingFromTheSnapshotAreRemoved) {
    go_to_race();
    push(snapshot({car(MY_ID, 1, 1, 100), car(OTHER_ID, 2, 2, 100)}));
    logic.update_race();
    ASSERT_EQ(state().players.size(), 1u);

    push(snapshot({car(MY_ID, 1, 1, 100)}));
    logic.update_race();

    EXPECT_TRUE(state().players.empty());
}

TEST_F(GameLogicTest, OtherPlayersAreAddedToTheSoundInfo) {
    go_to_race();
    push(snapshot({car(MY_ID, 1, 1, 100), car(OTHER_ID, 2, 2, 100)}));
    logic.update_race();

    push(GameUpdateDTO(GameLoopAction::START_SHOP, uint8_t{1}));
    logic.update_waiting();

    EXPECT_EQ(view.sound_ids, (std::vector<uint32_t>{MY_ID, OTHER_ID}));
}


// Leaderboards

TEST_F(GameLogicTest, LeaderboardIsKeptAndShown) {
    push(GameUpdateDTO(GameLoopAction::SEND_LEADERBOARD,
                       LeaderboardDTO("race", {PlayerRaceStatsDTO(MY_ID, "me", 1, 8, 30)})));

    EXPECT_TRUE(logic.update_leaderboard());

    EXPECT_EQ(logic.get_leaderboard().race_name, "race");
    ASSERT_EQ(logic.get_leaderboard().players.size(), 1u);
    EXPECT_EQ(view.leaderboard_opened, 1);
}

TEST_F(GameLogicTest, NextRaceGoesBackToTheShop) {
    push(GameUpdateDTO(GameLoopAction::START_SHOP, uint8_t{1}));

    EXPECT_FALSE(logic.update_leaderboard());

    EXPECT_EQ(state().phase, Phase::UPGRADE);
}

TEST_F(GameLogicTest, GameFinishedClosesTheRaceLeaderboard) {
    push(GameLoopAction::GAME_FINISHED);

    EXPECT_FALSE(logic.update_leaderboard());

    EXPECT_EQ(state().phase, Phase::GAME_FINISHED);
    EXPECT_EQ(view.leaderboard_closed, 1);
}

TEST_F(GameLogicTest, FinalLeaderboardIsKeptAndShown) {
    push(GameUpdateDTO(GameLoopAction::SEND_LEADERBOARD, LeaderboardDTO("Final Leaderboard")));

    EXPECT_TRUE(logic.update_game_finished());

    EXPECT_EQ(logic.get_leaderboard().race_name, "Final Leaderboard");
    EXPECT_EQ(view.leaderboard_opened, 1);
}

TEST_F(GameLogicTest, ReturningToTheLobbyClosesTheLeaderboard) {
    push(GameLoopAction::RETURN_TO_LOBBY);

    EXPECT_TRUE(logic.update_game_finished());

    EXPECT_EQ(view.leaderboard_closed, 1);
}
