#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H

#include <string>

struct LobbyDTO {
    int lobby_id; // id of the owner player
    int owner_id;
    std::string lobby_name;
    int max_players;

    LobbyDTO(): lobby_id(-1), owner_id(-1), lobby_name(""), max_players(0) {}

    LobbyDTO(int id, int owner, const std::string& name, int max_p):
        lobby_id(id), owner_id(owner), lobby_name(name), max_players(max_p) {}
};

#endif // LOBBY_DTO_H