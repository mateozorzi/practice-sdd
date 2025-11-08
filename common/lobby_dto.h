#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H

#include <string>

struct LobbyDTO {
    int lobby_id; // id of the owner player
    std::string owner;
    std::string lobby_name;
    int max_players;

    LobbyDTO(): lobby_id(-1), owner(""), lobby_name(""), max_players(0) {}

    LobbyDTO(int id, const std::string& owner, const std::string& name, int max_p):
        lobby_id(id), owner(owner), lobby_name(name), max_players(max_p) {}
};

#endif // LOBBY_DTO_H