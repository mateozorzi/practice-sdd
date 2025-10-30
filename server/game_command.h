#ifndef GAME_COMMAND_H
#define GAME_COMMAND_H


#include <cstdint>

#include "../common/movement_dto.h"

#include "client_request.h"


class GameCommand {


    uint32_t client_id;
    ClientRequest client_request;
    MovementDTO movement_dto;


public:
    GameCommand();
    explicit GameCommand(uint32_t client_id, const ClientRequest& client_request, const MovementDTO& movement_dto);

    uint32_t get_client_id() const;
    ClientRequest get_client_request() const;
    MovementDTO get_movement_dto() const;
};


#endif