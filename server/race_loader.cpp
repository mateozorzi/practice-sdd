#include "race_loader.h"

#include <fstream>
#include <iostream>


RaceLoader::RaceLoader(const ServerConfig& server_config):
        server_config(server_config), city_utils(server_config) {}


std::string RaceLoader::load_json_file(const std::string& file_path) {

    std::ifstream file(file_path);

    if (!file.is_open())
        return "[]";

    std::stringstream stream;
    stream << file.rdbuf();

    return stream.str();
}


RaceDTO RaceLoader::load_single_race(const std::filesystem::path& file_path) {

    std::ifstream file(file_path);
    std::string line;

    // Ignoring first 3 lines of the .csv files (metadata).
    std::getline(file, line);
    std::getline(file, line);
    std::getline(file, line);

    std::stringstream string_stream(line);

    std::string city_name;
    std::string number_of_laps_string;

    std::getline(string_stream, city_name, ',');
    std::getline(string_stream, number_of_laps_string, ',');

    uint8_t number_of_laps = static_cast<uint8_t>(std::stoi(number_of_laps_string));

    std::getline(file, line);

    std::vector<CheckpointDTO> checkpoints;

    while (std::getline(file, line)) {

        std::stringstream string_stream_2(line);
        std::string checkpoint;
        std::vector<std::string> row;
        while (std::getline(string_stream_2, checkpoint, ',')) row.push_back(checkpoint);

        checkpoints.emplace_back(CheckpointDTO(static_cast<uint8_t>(std::stoi(row[ID])),
                                               static_cast<char>(row[ORIENTATION].front()),
                                               static_cast<uint16_t>(std::stoi(row[X_CENTER])),
                                               static_cast<uint16_t>(std::stoi(row[Y_CENTER])),
                                               static_cast<uint8_t>(std::stoi(row[WIDTH])),
                                               static_cast<uint8_t>(std::stoi(row[HEIGHT]))));
    }

    const std::string& normalized_city_name = city_utils.normalize_city_name(city_name);

    const CityLoaderConfig& city_loader_config =
            city_utils.obtain_city_loader_config(normalized_city_name);

    const std::string& bridges_data = load_json_file(city_loader_config.objects.bridge_file);
    const std::string& decorations_data =
            load_json_file(city_loader_config.objects.decoration_file);

    return RaceDTO(file_path.stem().string(), city_name, checkpoints, number_of_laps, bridges_data,
                   decorations_data);
}


std::vector<RaceDTO> RaceLoader::load_races() {

    std::vector<RaceDTO> races;

    if (!std::filesystem::exists(server_config.world.races_folder))
        return races;

    for (const auto& entry: std::filesystem::directory_iterator(server_config.world.races_folder)) {

        if (entry.path().extension() == ".csv")
            races.push_back(load_single_race(entry.path()));
    }

    return races;
}
