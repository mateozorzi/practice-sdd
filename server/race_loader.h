#ifndef RACE_LOADER_H
#define RACE_LOADER_H


#include <filesystem>
#include <string>
#include <vector>

#include "../common/race_dto.h"

#include "city_utils.h"
#include "server_constants.h"


class RaceLoader {

    const ServerConfig& server_config;
    CityUtils city_utils;

    std::string load_json_file(const std::string& file_path);
    RaceDTO load_single_race(const std::filesystem::path& file_path);


public:
    RaceLoader(const RaceLoader&) = delete;
    RaceLoader& operator=(const RaceLoader&) = delete;

    explicit RaceLoader(const ServerConfig& server_config);

    std::vector<RaceDTO> load_races();
};


#endif
