#include "parameters.h"
#include "logger.h"
#include <boost/asio.hpp>
#include "connections.h"

#include <iostream>

int main(int argc, char *argv[]) {
    try {
        ClientParameters p;
        if (!p.read_program_arguments(argc, argv)) {
            return 0;
        }

        boost::asio::io_context io_context;
        Client client(io_context, p);
        io_context.run();
    } catch (std::exception &e) {
        Logger::print_error(e.what());
        return 1;
    }

    return 0;
}