#ifndef GAME_COMMAND_DTO_H
#define GAME_COMMAND_DTO_H


#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "../common/client_event_dto.h"

#include "car_dto.h"
#include "constants.h"
#include "movement_dto.h"
#include "player_dto.h"
#include "player_race_stats_dto.h"
#include "upgrade_dto.h"


struct GameCommandDTO {


    ClientAction client_action;
    std::variant<std::monostate, MovementDTO, PlayerDTO, CarDTO, std::string, PlayerRaceStatsDTO,
                 BuyUpgradeDTO>
            command_data;


    explicit GameCommandDTO(
            const ClientAction client_action = ClientAction::DISCONNECT,
            const std::variant<std::monostate, MovementDTO, PlayerDTO, CarDTO, std::string,
                               PlayerRaceStatsDTO, BuyUpgradeDTO>& command_data = std::monostate());

    template <typename T>
    std::optional<std::reference_wrapper<const T>> get_data() const {

        if (std::holds_alternative<T>(command_data))
            return std::cref(std::get<T>(command_data));

        return std::nullopt;
    }
};


#endif
