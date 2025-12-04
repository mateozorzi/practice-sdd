#ifndef RACE_LOADER_H
#define RACE_LOADER_H


#include <filesystem>
#include <string>
#include <vector>

#include "../common/race_dto.h"


constexpr uint8_t ID = 0;
constexpr uint8_t ORIENTATION = 1;
constexpr uint8_t X_CENTER = 2;
constexpr uint8_t Y_CENTER = 3;
constexpr uint8_t WIDTH = 4;
constexpr uint8_t HEIGHT = 5;


class RaceLoader {


    RaceDTO load_single_race(const std::filesystem::path& path);

    const std::string race_folder;


public:
    RaceLoader(const RaceLoader&) = delete;
    RaceLoader& operator=(const RaceLoader&) = delete;

    explicit RaceLoader(const std::string& race_folder);

    std::vector<RaceDTO> load_races();
};


#endif
