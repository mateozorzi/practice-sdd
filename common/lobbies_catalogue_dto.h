#ifndef LOBBIES_CATALOGUE_DTO_H
#define LOBBIES_CATALOGUE_DTO_H

#include <vector>
#include "lobby_dto.h"
#include <cstdint>

struct LobbyCatalogueDTO {
    uint8_t size;
    std::vector<LobbyDTO> lobbies;

    LobbyCatalogueDTO(): size(0), lobbies() {}

    explicit LobbyCatalogueDTO(const std::vector<LobbyDTO>& lobbies):
        size(static_cast<uint8_t>(lobbies.size())), lobbies(lobbies) {}
};

#endif // LOBBIES_CATALOGUE_DTO_H