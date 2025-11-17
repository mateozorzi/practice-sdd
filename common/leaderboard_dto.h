#ifndef LEADERBOARD_DTO_H
#define LEADERBOARD_DTO_H

#include <cstdint>
#include <vector>
#include "player_race_stats_dto.h"

struct LeaderboardDTO{
    uint8_t n_players;
    std::vector<PlayerRaceStatsDTO> players;

    LeaderboardDTO(const std::vector<PlayerRaceStatsDTO>& players)
    : n_players(static_cast<uint8_t>(players.size())), players(players) {}

    LeaderboardDTO() : n_players(0), players() {}

};


#endif
