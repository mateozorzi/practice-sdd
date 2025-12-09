#include "lobby_dto.h"


LobbyDTO::LobbyDTO(const LobbyAction action, const uint32_t lobby_id, const std::string& lobby_name,
                   const uint8_t max_players, const uint8_t active_players,
                   const RaceCatalogueDTO& selected_races):
        action(action),
        lobby_id(lobby_id),
        lobby_name(lobby_name),
        max_players(max_players),
        active_players(active_players),
        selected_races(selected_races) {}
