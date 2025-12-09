#include "lobby_catalogue_dto.h"


LobbyCatalogueDTO::LobbyCatalogueDTO(const std::vector<LobbyDTO>& lobbies):
            size(static_cast<uint8_t>(lobbies.size())), lobbies(lobbies) {}
