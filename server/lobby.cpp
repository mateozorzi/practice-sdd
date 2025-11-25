#include "lobby.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

#include "../common/race_catalogue_dto.h"


Lobby::Lobby(const ServerConfig& config): config(config) { initialize_races(); }

void Lobby::initialize_races() {
    // races are in races/ folder
    std::string path_folder = "server/races/";

    // if folder does not exist, there is no races to load
    if (!std::filesystem::exists(path_folder)) {
        std::cerr << "[" << LOBBY << "] "
                  << "Race folder not found, no races to load." << std::endl;
        return;
    }

    // folder exists, load races
    for (const auto& entry: std::filesystem::directory_iterator(path_folder)) {
        if (entry.path().extension() == ".csv") {
            std::cout << "Loading race file: " << entry.path().filename() << std::endl;

            std::ifstream file(entry.path());
            std::string line;

            // editor info: skip
            std::getline(file, line);
            // headers: City, n_laps: skip
            std::getline(file, line);

            // info city and number of laps
            std::getline(file, line);
            std::stringstream ss_(line);
            std::string cell_;
            std::vector<std::string> row_;

            while (std::getline(ss_, cell_, ',')) {
                row_.push_back(cell_);
            }

            std::string city = row_[0];
            uint8_t n_laps = static_cast<uint8_t>(std::stoi(row_[1]));

            // header: id, orientation, xCenter, yCenter, height, width,: skip
            std::getline(file, line);
            std::vector<CheckpointDTO> checkpoints;
            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string cell;
                std::vector<std::string> row;

                while (std::getline(ss, cell, ',')) {
                    row.push_back(cell);
                }

                uint8_t id = static_cast<uint8_t>(std::stoi(row[0]));
                char orientation = row[1][0];
                uint16_t x = static_cast<uint16_t>(std::stoi(row[2]));
                uint16_t y = static_cast<uint16_t>(std::stoi(row[3]));
                uint8_t height = static_cast<uint8_t>(std::stoi(row[4]));
                uint8_t width = static_cast<uint8_t>(std::stoi(row[5]));

                CheckpointDTO c(id, orientation, x, y, width, height);
                checkpoints.push_back(c);
            }

            // race name is file name withou extension
            std::string race_name = entry.path().stem().string();

            RaceDTO race(race_name, city, checkpoints, n_laps);

            std::cout << "Loaded race: " << race.name << " in " << race.city << " with "
                      << race.checkpoints.size() << " checkpoints and " << (int)race.laps
                      << " laps." << std::endl;

            races.push_back(race);
        }
    }
}

std::pair<Queue<GameCommand>&, Queue<GameUpdateDTO>&> Lobby::create_new_game(
        const uint32_t client_id, const std::string& username, const uint8_t max_players,
        const std::string& name, RaceCatalogueDTO& selected) {

    std::lock_guard lock(mtx);

    uint32_t game_id = client_id;

    auto game = std::make_unique<Game>(game_id, max_players, config, name, selected);

    std::cerr << "[" << LOBBY
              << "] "
                 "Creating new game with ID: "
              << game_id << " | Max players: " << (int)max_players
              << " | (Owner has the same id): " << client_id << std::endl;

    auto queues = game->add_player(client_id, username);
    games.emplace(game_id, std::move(game));

    return queues;
}

std::pair<Queue<GameCommand>&, Queue<GameUpdateDTO>&> Lobby::join_existing_game(
        const uint32_t client_id, const uint32_t game_id, const std::string& username) {

    std::lock_guard lock(mtx);

    auto it = games.find(game_id);

    if (it == games.end()) {

        std::cerr << "[" << LOBBY
                  << "] "
                     "Game "
                  << (int)game_id << " not found." << std::endl;
        static Queue<GameCommand> dummy_command;
        static Queue<GameUpdateDTO> dummy_snapshot;
        return {dummy_command, dummy_snapshot};
    }

    Game& game = *(it->second);
    if (game.is_full()) {

        std::cerr << "[" << LOBBY << "] "
                  << "Game with ID: " << game_id << " is full." << std::endl;
        static Queue<GameCommand> dummy_command;
        static Queue<GameUpdateDTO> dummy_snapshot;
        return {dummy_command, dummy_snapshot};
    }

    auto queues = game.add_player(client_id, username);

    return queues;
}

LobbyCatalogueDTO Lobby::list_games() {

    std::lock_guard lock(mtx);

    std::vector<LobbyDTO> lobby_dtos;

    for (const auto& [game_id, game_ptr]: games) {

        if (!game_ptr->has_started())
            lobby_dtos.emplace_back(LobbyAction::INFO_LOBBY, game_id, game_ptr->get_name(),
                                    game_ptr->get_max_players(), game_ptr->get_active_players());
    }

    return LobbyCatalogueDTO(std::move(lobby_dtos));
}

void Lobby::reap() {

    std::lock_guard lock(mtx);

    for (auto it = games.begin(); it != games.end();) {

        const Game& game = *it->second;
        if (game.is_empty()) {
            std::cerr << "[" << LOBBY << "] "
                      << "Game with ID: " << (int)it->first << " is now empty, removing it."
                      << std::endl;
            it = games.erase(it);
        } else {
            ++it;
        }
    }
}

void Lobby::remove_player(const uint32_t client_id, const uint32_t game_id) {

    std::lock_guard lock(mtx);

    const auto it = games.find(game_id);

    if (it == games.end())
        return;

    Game& game = *it->second;
    game.remove_player(client_id);
}

RaceCatalogueDTO Lobby::list_races() {
    std::lock_guard lock(mtx);

    return RaceCatalogueDTO(std::move(races));
}
