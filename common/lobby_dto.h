#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H

#include <string>

#include "../common/race_catalogue_dto.h"

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
    uint32_t lobby_id;  // id of the owner player
    std::string lobby_name;
    uint8_t max_players;
    uint8_t active_players;
    RaceCatalogueDTO selected_races;


    LobbyDTO():
            action(LobbyAction::CREATE_LOBBY),
            lobby_id(-1),
            lobby_name(""),
            max_players(0),
            active_players(0) {}

    // create lobby
    explicit LobbyDTO(LobbyAction act, const uint32_t id, const std::string& name,
                      const uint8_t max_p):
            action(act), lobby_id(id), lobby_name(name), max_players(max_p), active_players(0) {}

    explicit LobbyDTO(LobbyAction act, const uint32_t id, const std::string& name,
                      const uint8_t max_p, const uint8_t active_p):
            action(act),
            lobby_id(id),
            lobby_name(name),
            max_players(max_p),
            active_players(active_p) {}

    explicit LobbyDTO(LobbyAction act, const uint32_t id, const std::string& name,
                      const uint8_t max_p, const uint8_t active_p, const RaceCatalogueDTO& races):
            action(act),
            lobby_id(id),
            lobby_name(name),
            max_players(max_p),
            active_players(active_p),
            selected_races(races) {}

    // join Lobby, start, leave
    explicit LobbyDTO(LobbyAction act, const uint32_t id):
            action(act), lobby_id(id), lobby_name(""), max_players(0), active_players(0) {}

    explicit LobbyDTO(LobbyAction act):
            action(act), lobby_id(-1), lobby_name(""), max_players(0), active_players(0) {}

    LobbyDTO(LobbyAction act, const std::string& name):
            action(act), lobby_id(-1), lobby_name(name), max_players(0), active_players(0) {}

    explicit LobbyDTO(const uint32_t id):
            action(LobbyAction::INFO_LOBBY),
            lobby_id(id),
            lobby_name(""),
            max_players(0),
            active_players(0) {}
};

#endif  // LOBBY_DTO_H
