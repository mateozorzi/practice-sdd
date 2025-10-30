#include "game_command.h"


GameCommand::GameCommand(): client_id(0), client_request(ClientRequest::UNKNOWN), movement_dto(Direction::UP) {}

GameCommand::GameCommand(const uint32_t client_id, const ClientRequest& client_request, const MovementDTO& movement_dto):
        client_id(client_id), client_request(client_request), movement_dto(movement_dto) {}


uint32_t GameCommand::get_client_id() const { return client_id; }

ClientRequest GameCommand::get_client_request() const { return client_request; }

MovementDTO GameCommand::get_movement_dto() const { return movement_dto; }

