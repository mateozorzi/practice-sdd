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
    // variant with possible data types
    std::variant<std::monostate, WorldSnapshotDTO, RaceDTO, LeaderboardDTO, uint8_t> data;

    GameUpdateDTO(
            GameLoopAction gameloop_action,
            const std::variant<std::monostate, WorldSnapshotDTO, RaceDTO, LeaderboardDTO, uint8_t>& data):
            gameloop_action(gameloop_action), data(data) {}

    // default constructor
    GameUpdateDTO():
            gameloop_action(GameLoopAction::SEND_UPDATE_POSITIONS), data(std::monostate{}) {}
};


#endif
