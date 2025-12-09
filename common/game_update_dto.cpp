#include "game_update_dto.h"


GameUpdateDTO::GameUpdateDTO(const GameLoopAction gameloop_action,
                             const std::variant<std::monostate, WorldSnapshotDTO, RaceDTO,
                                                LeaderboardDTO, uint8_t, uint32_t>& data):
        gameloop_action(gameloop_action), data(data) {}
