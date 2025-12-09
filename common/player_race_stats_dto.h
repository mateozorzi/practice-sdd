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


    explicit PlayerRaceStatsDTO(uint32_t client_id = 0, const std::string& username = "",
                                uint8_t position = 0, uint8_t points = 0, uint32_t total_time = 0);
};


#endif
