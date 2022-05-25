#ifndef ROBOTS_UTILS_H
#define ROBOTS_UTILS_H

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

namespace Checker {
    void check_errno();
    void ensure(bool x);
    void check_with_errno(int x);
}

namespace Utils {
    uint64_t read_number(std::string &number_str, uint64_t min_limit,
                         uint64_t max_limit, std::string &name);
}


#endif //ROBOTS_UTILS_H
