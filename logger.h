#ifndef ROBOTS_LOGGER_H
#define ROBOTS_LOGGER_H

#include "string"
#include <iostream>

class Logger {
private:
#ifdef NDEBUG
    static constexpr auto debug_compile = false;
#else
    static constexpr auto debug_compile = true;
#endif

public:
    template<typename... Args>
    static void print_debug(Args &&...args) {
        if (debug_compile) {
            (std::cerr << ... << args);
            std::cerr << "\n";
        }
    }

    template<typename... Args>
    static void print_info(Args &&...args) {
        (std::cout << ... << args);
        std::cout << "\n";
    }

    template<typename... Args>
    static void print_error(Args &&...args) {
        std::cerr << "Error: ";
        (std::cerr << ... << args);
        std::cerr << "\n";
    }
};

#endif //ROBOTS_LOGGER_H
