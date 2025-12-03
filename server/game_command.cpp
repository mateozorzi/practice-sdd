#include "game_command.h"

#include <utility>


GameCommand::GameCommand():
        player_id(0), client_action(ClientAction::DISCONNECT), command_data(std::monostate{}) {}


GameCommand::GameCommand(uint32_t player_id, GameCommandDTO&& dto):
        player_id(player_id),
        client_action(dto.client_action),
        command_data(std::move(dto.command_data)) {}


uint32_t GameCommand::get_player_id() const { return player_id; }

ClientAction GameCommand::get_client_action() const { return client_action; }
