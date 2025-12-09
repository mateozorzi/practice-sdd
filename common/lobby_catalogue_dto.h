#ifndef LOBBY_CATALOGUE_DTO_H
#define LOBBY_CATALOGUE_DTO_H


#include <cstdint>
#include <vector>

#include "lobby_dto.h"


struct LobbyCatalogueDTO {


    uint8_t size;
    std::vector<LobbyDTO> lobbies;


    explicit LobbyCatalogueDTO(const std::vector<LobbyDTO>& lobbies = {});
};


#endif
