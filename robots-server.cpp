#include "parameters.h"
#include "logger.h"
#include "connections.h"
#include <boost/asio.hpp>

#include <iostream>

int main(int argc, char *argv[]) {
    try {
        ServerParameters p;
        if (!p.read_program_arguments(argc, argv)) {
            return 0;
        }

        boost::asio::io_context io_context;
        Server server(io_context, p);
        io_context.run();
    } catch (std::exception &e) {
        Logger::print_error(e.what());
        return 1;
    }

    return 0;
}

//./robots-server --port 54321 --server-name 'Test' --players-count 1 --turn-duration 1000 --game-length 10 --size-x 4 --size-y 4 --explosion-radius 2 --bomb-timer 3 --initial-blocks 6

