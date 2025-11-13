#include "game_command.h"

#include <utility>


GameCommand::GameCommand():
        client_id(0), client_action(ClientAction::DISCONNECT), command_data(std::monostate{}) {}


GameCommand::GameCommand(uint32_t client_id, GameCommandDTO&& dto):
        client_id(client_id),
        client_action(dto.client_action),
        command_data(std::move(dto.command_data)) {}


uint32_t GameCommand::get_client_id() const { return client_id; }

ClientAction GameCommand::get_client_action() const { return client_action; }
