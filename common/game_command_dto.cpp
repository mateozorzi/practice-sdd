#include "game_command_dto.h"


GameCommandDTO::GameCommandDTO(
        const ClientAction client_action,
        const std::variant<std::monostate, MovementDTO, PlayerDTO, CarDTO, std::string,
                           PlayerRaceStatsDTO, BuyUpgradeDTO>& command_data):
        client_action(client_action), command_data(command_data) {}
