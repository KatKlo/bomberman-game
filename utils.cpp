#include "utils.h"
#include <cstring>
#include <iostream>
#include <chrono>

namespace Checker {
    void check_errno() {
        if (errno != 0) {
            Logger::print_error("Error: errno ", errno, " in ", __func__,
                                " at ", __FILE__, ":", __LINE__,
                                "\n", strerror(errno));
        }
    }

    void ensure(bool x) {
        if (!x) {
            Logger::print_error("Error: statement was false in ", __func__, " at ", __FILE__, ":", __LINE__);
        }
    }

    void check_with_errno(int x) {
        if (x != 0) {
            Checker::check_errno();
            exit(EXIT_FAILURE);
        }
    }
}

//namespace Utils {
//    uint64_t read_number(std::string &number_str, uint64_t min_limit,
//                         uint64_t max_limit, std::string &name) {
//        errno = 0;
//        char *end;
//
//        unsigned long number = strtoull(number_str.c_str(), &end, 10);
//
//        if (*end != '\0') {
//            errno = EINVAL;
//        }
//        Checker::check_errno();
//        if (number > max_limit || number < min_limit) {
//            Logger::print_error(number, " is not a valid", name, " number");
//        }
//
//        return number;
//    }
//}
