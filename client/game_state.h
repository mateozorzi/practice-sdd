#ifndef GAME_STATE_H
#define GAME_STATE_H
#include <string>
#include <unordered_map>

#include "../common/phase.h"
#include "/common/phase.h"

struct PlayerInfo {
    int x = 0;
    int y = 0;
    int angle = 0;
    int life = 0;
    int time = 0;
    std::string model;
    bool isMoving = false;
    bool died = false;
    bool crashed = false;
};

struct LocalInfo {
    std::string username;
    std::unordered_map<std::string, PlayerInfo> players;
    PlayerInfo player;
    Phase phase = Phase::WAITING_PLAYERS;
};
#endif
