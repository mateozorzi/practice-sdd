#include "leaderboard_dto.h"


LeaderboardDTO::LeaderboardDTO(const std::string& race_name,
                               const std::vector<PlayerRaceStatsDTO>& players):
        race_name(race_name), n_players(static_cast<uint8_t>(players.size())), players(players) {}
