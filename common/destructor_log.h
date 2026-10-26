#ifndef DESTRUCTOR_LOG_H_
#define DESTRUCTOR_LOG_H_

#include <iostream>
#include <string_view>

#include "constants.h"

// Logs for the destructors of the threads. They never throw, so a destructor can call them
// without letting an exception escape (and terminate the program).

inline void log_stopping(std::string_view component) noexcept {
    try {
        std::cerr << "[" << component << "] Stopping." << std::endl;
    } catch (...) {}
}

inline void log_destructor_error(std::string_view component, std::string_view what) noexcept {
    try {
        std::cerr << "[" << component << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << what;
    } catch (...) {}
}

#endif
