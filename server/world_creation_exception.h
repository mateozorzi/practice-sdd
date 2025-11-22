#ifndef WORLD_CREATION_EXCEPTION_H
#define WORLD_CREATION_EXCEPTION_H


#include <stdexcept>
#include <string>


class WorldCreationException: public std::runtime_error {

public:
    WorldCreationException(const WorldCreationException&) = delete;
    WorldCreationException& operator=(const WorldCreationException&) = delete;

    explicit WorldCreationException(const std::string& message): std::runtime_error(message) {}
};


#endif