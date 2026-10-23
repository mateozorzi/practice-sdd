#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H


#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/game_update_dto.h"
#include "../common/leaderboard_dto.h"
#include "../common/queue.h"
#include "../common/world_snapshot_dto.h"

#include "game_state.h"


// Screen changes the game logic asks for. The SDL implementation lives in Game.
class GameView {

public:
    virtual void set_sound_info(const std::vector<uint32_t>& ids) = 0;
    virtual void load_city(const std::string& city_name, const std::string& bridges_data,
                           const std::string& decorations_data) = 0;

    virtual void open_shop() = 0;
    virtual void close_shop() = 0;

    virtual void open_leaderboard() = 0;
    virtual void close_leaderboard() = 0;

    // The own player collected a power-up
    virtual void play_power_up_sound() = 0;

    virtual ~GameView() = default;
};


// Applies the server updates to the local game state, one method per game phase. Each
// update_* method returns false when the phase has to change.
class GameLogic {

    GameView& view;
    Queue<GameUpdateDTO>& updates;
    LocalInfo game_state;
    LeaderboardDTO leaderboard;
    std::string current_city = "";
    bool keep_running = true;

    void reset_events();

    static void detect_events_for_player(PlayerInfo& info, const PlayerDTO& dto,
                                         const CarPositionDTO& car_pos);

    void update_player(const PlayerDTO& player, const CarPositionDTO& car,
                       const std::unordered_map<uint32_t, PlayerInfo>& prev_state);

    bool collected_by_me(const PowerUpDTO& power_up) const;
    void detect_own_power_up_pickups(const std::vector<PowerUpDTO>& power_ups);

    void update_game_state_from_snapshot(const WorldSnapshotDTO& snapshot);


public:
    GameLogic(GameView& view, Queue<GameUpdateDTO>& updates, uint32_t id);

    bool update_waiting();
    bool update_upgrade();
    bool update_countdown();
    bool update_race();
    bool update_leaderboard();
    bool update_game_finished();

    LocalInfo& get_state();
    const LeaderboardDTO& get_leaderboard() const;

    bool is_running() const;
    void stop();
};


#endif
