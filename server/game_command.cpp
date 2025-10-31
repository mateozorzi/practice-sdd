#include "game_command.h"


GameCommand::GameCommand():
        client_id(0), client_request(ClientRequest::UNKNOWN), command_data(std::monostate{}) {}

GameCommand::GameCommand(const uint32_t id, const ClientRequest& client_request)
        : client_id(id), client_request(client_request), command_data(std::monostate{}) {}

GameCommand::GameCommand(const uint32_t client_id, const ClientRequest& client_request,
                        const MovementDTO& movement_dto)
: client_id(client_id), client_request(client_request), command_data(movement_dto) {}

GameCommand::GameCommand(const uint32_t client_id, const ClientRequest& client_request,
                        const PlayerDTO& player_dto)
: client_id(client_id), client_request(client_request), command_data(player_dto) {}


uint32_t GameCommand::get_client_id() const { return client_id; }

ClientRequest GameCommand::get_client_request() const { return client_request; }
