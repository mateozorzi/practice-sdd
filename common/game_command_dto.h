#ifndef GAME_COMMAND_DTO_H
#define GAME_COMMAND_DTO_H


#include <variant>
#include <optional>
#include <functional>
#include <string>
#include <cstdint>

#include "constants.h"
#include "car_dto.h"
#include "movement_dto.h"
#include "player_dto.h"
#include "player_race_stats_dto.h"
#include "upgrade_dto.h"


struct GameCommandDTO {
    ClientAction client_action;
    std::variant<std::monostate, MovementDTO, PlayerDTO, CarDTO, std::string, PlayerRaceStatsDTO, UpgradeDTO> command_data;

    GameCommandDTO() 
    : client_action(ClientAction::DISCONNECT), 
        command_data(std::monostate{}) {}

    explicit GameCommandDTO(ClientAction client_action) 
    : client_action(client_action), 
        command_data(std::monostate{}) {}
    
    template<typename T>
    GameCommandDTO(ClientAction client_action, T&& data) 
        : client_action(client_action), 
          command_data(std::forward<T>(data)) {}

    template <typename T>
    std::optional<std::reference_wrapper<const T>> get_data() const {
        if (std::holds_alternative<T>(command_data))
            return std::cref(std::get<T>(command_data));
        return std::nullopt;
    }
    
    
};


#endif
