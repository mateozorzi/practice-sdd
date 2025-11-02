#include <stdexcept>

class WorldCreationException: public std::runtime_error {
public:
    explicit WorldCreationException(const std::string& message): std::runtime_error(message) {}
};