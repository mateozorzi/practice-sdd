#ifndef GAME_STATE_H
#define GAME_STATE_H
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/npc_dto.h"
#include "../common/phase.h"


struct PlayerInfo {

    uint32_t id;
    int x = 0;
    int y = 0;
    int angle = 0;
    int life = 0;
    int max_life = 100;
    int time = 0;
    std::string model;
    bool isMoving = false;
    bool died = false;
    bool crashed = false;
    CarLayer layer = CarLayer::ON_GROUND;
    CheckpointDTO checkpoint = CheckpointDTO{};
    bool finish_race = false;
};

struct LocalInfo {

    uint32_t id;
    std::unordered_map<uint32_t, PlayerInfo> players;
    PlayerInfo player;
    std::vector<NpcDTO> npcs;
    Phase phase = Phase::WAITING_FOR_CAR_SELECTION;
};


#endif
