#include <fstream>


#include "race_loader.h"


RaceLoader::RaceLoader(const std::string& race_folder)
    : race_folder(race_folder) {}


RaceDTO RaceLoader::load_single_race(const std::filesystem::path& path) {

    std::ifstream file(path);
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
        while (std::getline(string_stream_2, checkpoint, ','))
            row.push_back(checkpoint);

        checkpoints.emplace_back(
            std::stoi(row[ID]),
            row[ORIENTATION].front(),
            std::stoi(row[X_CENTER]),
            std::stoi(row[Y_CENTER]),
            std::stoi(row[HEIGHT]),
            std::stoi(row[WIDTH])
        );
    }

    return RaceDTO(path.stem().string(), city_name, checkpoints, number_of_laps);
}


std::vector<RaceDTO> RaceLoader::load_races() {

    std::vector<RaceDTO> races;

    if (!std::filesystem::exists(race_folder))
        return races;

    for (const auto& entry : std::filesystem::directory_iterator(race_folder)) {

        if (entry.path().extension() == ".csv")
            races.push_back(load_single_race(entry.path()));
    }

    return races;
}
