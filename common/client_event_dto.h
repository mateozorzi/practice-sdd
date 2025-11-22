#ifndef EVENT_DTO_H
#define EVENT_DTO_H
#include <variant>

#include "movement_dto.h"


enum ButtonType { Health, Speed, Acceleration, Handling, None };

struct BuyUpgradeDTO {
    ButtonType upgrade;
};

using ClientEventDTO = std::variant<MovementDTO, BuyUpgradeDTO>;


#endif
