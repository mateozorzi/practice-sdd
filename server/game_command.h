#ifndef GAME_COMMAND_H
#define GAME_COMMAND_H


#include <cstdint>
#include <functional>
#include <optional>
#include <variant>

#include "../common/car_dto.h"
#include "../common/movement_dto.h"
#include "../common/player_dto.h"

#include "client_request.h"


class GameCommand {


    uint32_t client_id;
    ClientRequest client_request;
    std::variant<std::monostate, MovementDTO, PlayerDTO, car_dto> command_data;


public:
    GameCommand();

    explicit GameCommand(uint32_t client_id, const ClientRequest& client_request);
    explicit GameCommand(uint32_t client_id, const ClientRequest& client_request,
                         const MovementDTO& movement_dto);
    explicit GameCommand(uint32_t client_id, const ClientRequest& client_request,
                         const PlayerDTO& player_dto);
    explicit GameCommand(uint32_t client_id, const ClientRequest& client_request,
                         const car_dto& car_dto);

    uint32_t get_client_id() const;
    ClientRequest get_client_request() const;

    template <typename T>
    std::optional<std::reference_wrapper<const T>> get_data() const {

        if (std::holds_alternative<T>(command_data))
            return std::cref(std::get<T>(command_data));

        return std::nullopt;
    }
};


#endif
