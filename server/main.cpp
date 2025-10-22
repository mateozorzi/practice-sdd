#include <cstdint>
#include <iostream>
#include <string>


constexpr uint8_t SUCCESS = 0;
constexpr uint8_t ERROR = 1;
constexpr uint8_t SERVER_PORT = 1;
constexpr uint8_t SERVER_ARGUMENTS = 2;

constexpr std::string_view SERVER_MAIN = "SERVER_MAIN";
constexpr std::string_view ERROR_ARGUMENTS_SERVER = "1 argument is needed: <service_name or port>";
constexpr std::string_view UNKNOWN_EXCEPTION = "Unknown exception";


int main(const int argc, const char* const argv[]) {

    if (argc != SERVER_ARGUMENTS) {

        std::cerr << "[" << SERVER_MAIN << "] " << ERROR_ARGUMENTS_SERVER << std::endl;; 
        return ERROR;
    }

    const std::string port = argv[SERVER_PORT];

    try {

        // Server server(port);
        // server.run();
    } catch (const std::exception& e) {

        std::cerr << "[" << SERVER_MAIN << "] " << e.what() << std::endl;
        return ERROR;
    } catch (...) {

        std::cerr << "[" << SERVER_MAIN << "] " << UNKNOWN_EXCEPTION << std::endl;
        return ERROR;
    }

    return SUCCESS;
}
