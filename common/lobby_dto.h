#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H


#include <string>

#include "race_catalogue_dto.h"


enum class LobbyAction : uint8_t {

    CREATE_LOBBY = 0x00,
    JOIN_LOBBY = 0x01,
    SEND_LOBBY_LIST = 0x02,
    LEAVE_LOBBY = 0x03,
    START_LOBBY = 0x04,
    EXIT_GAME = 0x05,
    INFO_LOBBY = 0x06,
    CHANGE_USERNAME = 0x07,
    LIST_RACES = 0x08,
    SEND_RACE_SELECTION = 0x09,

    CREATE_OR_JOIN_SUCCESS = 0x10,
    CREATE_OR_JOIN_ERROR = 0x11,
    MIN_PLAYERS_ERROR = 0x12,
    MAX_PLAYERS_ERROR = 0x13,
};


struct LobbyDTO {


    LobbyAction action;
    uint32_t lobby_id;
    std::string lobby_name;
    uint8_t max_players;
    uint8_t active_players;
    RaceCatalogueDTO selected_races;


    explicit LobbyDTO(const LobbyAction action = LobbyAction::CREATE_LOBBY, uint32_t lobby_id = 0,
                      const std::string& lobby_name = "", uint8_t max_players = 0,
                      uint8_t active_players = 0,
                      const RaceCatalogueDTO& selected_races = RaceCatalogueDTO());
};

#endif
