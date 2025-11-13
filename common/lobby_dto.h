#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H

#include <string>

enum class LobbyAction : uint8_t {

    CREATE_LOBBY = 0x00,
    JOIN_LOBBY = 0x01,
    SEND_LOBBY_LIST = 0x02,
    LEAVE_LOBBY = 0x03,
    START_LOBBY = 0x04,
    EXIT_GAME = 0x05,
    INFO_LOBBY = 0x06,
    CHANGE_USERNAME = 0x07

};


struct LobbyDTO {
    LobbyAction action;
    int lobby_id;  // id of the owner player
    std::string lobby_name;
    int max_players;


    LobbyDTO(): action(LobbyAction::CREATE_LOBBY), lobby_id(-1), lobby_name(""), max_players(0) {}

    // create lobby
    explicit LobbyDTO(LobbyAction act, int id, const std::string& name, int max_p):
            action(act), lobby_id(id), lobby_name(name), max_players(max_p) {}

    // join Lobby, start, leave
    explicit LobbyDTO(LobbyAction act, int id):
            action(act), lobby_id(id), lobby_name(""), max_players(0) {}

    explicit LobbyDTO(LobbyAction act): action(act), lobby_id(-1), lobby_name(""), max_players(0) {}

    LobbyDTO(LobbyAction act, const std::string& name):
            action(act), lobby_id(-1), lobby_name(name), max_players(0) {}

    explicit LobbyDTO(const int id):
            action(LobbyAction::INFO_LOBBY), lobby_id(id), lobby_name(""), max_players(0) {}
};

#endif  // LOBBY_DTO_H