#include "player_race_stats_dto.h"


PlayerRaceStatsDTO::PlayerRaceStatsDTO(const uint32_t client_id, const std::string& username,
                                       const uint8_t position, const uint8_t points,
                                       const uint32_t total_time):
        client_id(client_id),
        username(username),
        position(position),
        points(points),
        total_time(total_time) {}
