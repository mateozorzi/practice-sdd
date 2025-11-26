#ifndef SERVER_CONFIG_LOADER_H
#define SERVER_CONFIG_LOADER_H


#include <string>

#include <yaml-cpp/yaml.h>

#include "server_config.h"


class ServerConfigLoader {


    void load_game_config(const YAML::Node& config, ServerConfig& server_config);

    void load_physics_config(const YAML::Node& config, ServerConfig& server_config);
    void load_car_physics(const YAML::Node& car_node, CarConfig& car_config);

    void load_world_config(const YAML::Node& config, ServerConfig& server_config);
    void load_city(const YAML::Node& node, CityLoaderConfig& city);


public:


    ServerConfigLoader() = default;

    ServerConfig load_yaml(const std::string& base_path);
};

#endif
