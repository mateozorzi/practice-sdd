#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <string>
#include "server_config.h"

class ConfigLoader {
public:

    static ServerConfig load_yaml(const std::string& filepath);

private:
    ConfigLoader() = default;
    ~ConfigLoader() = default;
};

#endif // CONFIG_LOADER_H
