#ifndef GAME_COMMAND_H
#define GAME_COMMAND_H


#include <cstdint>
#include <functional>
#include <optional>
#include <variant>

#include "../common/car_catalogue_dto.h"
#include "../common/constants.h"
#include "../common/game_command_dto.h"
#include "../common/player_race_stats_dto.h"


class GameCommand {


    uint32_t client_id;
    ClientAction client_action;
    std::variant<std::monostate, MovementDTO, PlayerDTO, car_dto, std::string, PlayerRaceStatsDTO> command_data;


public:
    // GameCommand(const GameCommand&) = delete;
    // GameCommand& operator=(const GameCommand&) = delete;

    GameCommand();

    explicit GameCommand(uint32_t client_id, GameCommandDTO&& dto);

    uint32_t get_client_id() const;
    ClientAction get_client_action() const;

    template <typename T>
    std::optional<std::reference_wrapper<const T>> get_data() const{

        if (std::holds_alternative<T>(command_data))
            return std::cref(std::get<T>(command_data));

        return std::nullopt;
    }
};


#endif
