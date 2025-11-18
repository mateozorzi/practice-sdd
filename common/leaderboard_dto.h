#ifndef LEADERBOARD_DTO_H
#define LEADERBOARD_DTO_H

#include <cstdint>
#include <vector>
#include "player_race_stats_dto.h"

struct LeaderboardDTO{
    std::string race_name;
    uint8_t n_players;
    std::vector<PlayerRaceStatsDTO> players;

    LeaderboardDTO(const std::string& race_name, const std::vector<PlayerRaceStatsDTO>& players)
    : race_name(race_name), n_players(static_cast<uint8_t>(players.size())), players(players) {}

    LeaderboardDTO() : race_name(""), n_players(0), players() {}

};


#endif
