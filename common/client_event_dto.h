#ifndef EVENT_DTO_H
#define EVENT_DTO_H
#include <variant>

#include "constants.h"
#include "movement_dto.h"


enum ButtonType { Health, Speed, Acceleration, Handling, None };

struct BuyUpgradeDTO {
    ButtonType upgrade;
};

struct ClientActionDTO {
    ClientAction action;

    explicit ClientActionDTO(ClientAction action): action(action) {}
};

using ClientEventDTO = std::variant<ClientActionDTO, MovementDTO, BuyUpgradeDTO>;


#endif
