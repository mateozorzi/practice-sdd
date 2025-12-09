#ifndef LEADERBOARD_DTO_H
#define LEADERBOARD_DTO_H


#include <cstdint>
#include <string>
#include <vector>

#include "player_race_stats_dto.h"


struct LeaderboardDTO {


    std::string race_name;
    uint8_t n_players;
    std::vector<PlayerRaceStatsDTO> players;


    explicit LeaderboardDTO(const std::string& race_name = "",
                            const std::vector<PlayerRaceStatsDTO>& players = {});
};


#endif
