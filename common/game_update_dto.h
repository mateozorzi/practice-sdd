#ifndef GAME_UPDATE_DTO_H
#define GAME_UPDATE_DTO_H


#include <cstdint>
#include <variant>

#include "constants.h"
#include "leaderboard_dto.h"
#include "race_dto.h"
#include "world_snapshot_dto.h"


struct GameUpdateDTO {


    GameLoopAction gameloop_action;
    std::variant<std::monostate, WorldSnapshotDTO, RaceDTO, LeaderboardDTO, uint8_t, uint32_t> data;


    explicit GameUpdateDTO(
            GameLoopAction gameloop_action = GameLoopAction::DISCONNECT,
            const std::variant<std::monostate, WorldSnapshotDTO, RaceDTO, LeaderboardDTO, uint8_t,
                               uint32_t>& data = std::monostate());
};


#endif
