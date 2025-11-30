#ifndef PHASE_H
#define PHASE_H

enum class Phase {
    WAITING_FOR_CAR_SELECTION = 0,
    UPGRADE = 1,
    COUNTDOWN = 2,
    IN_RACE = 3,
    LEADERBOARD = 4,
    GAME_FINISHED = 5,
};

#endif
