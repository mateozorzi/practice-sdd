#ifndef PLAYER_RACE_STATS_DTO_H
#define PLAYER_RACE_STATS_DTO_H


#include <cstdint>
#include <string>


struct PlayerRaceStatsDTO {

    uint32_t client_id;
    std::string username;
    uint8_t position;
    uint8_t points;
    uint32_t total_time;

    PlayerRaceStatsDTO(uint32_t client_id, const std::string& username, uint8_t position,
                       uint8_t points, uint32_t total_time):
            client_id(client_id),
            username(username),
            position(position),
            points(points),
            total_time(total_time) {}
};


#endif
