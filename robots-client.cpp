#include "parameters.h"
#include "utils.h"
#include <boost/asio.hpp>
#include "connections.h"

#include <iostream>

int main(int argc, char *argv[]) {
    try {
        ClientParameters p{argc, argv};

        boost::asio::io_context io_context;
        Client client(io_context, p);
        io_context.run();
    } catch (std::exception &e) {
        Logger::print_debug("error catched in main", e.what());
    }
    return 0;
}