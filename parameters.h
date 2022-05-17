#ifndef ROBOTS_PARAMETERS_H
#define ROBOTS_PARAMETERS_H

#include <iostream>
#include <boost/program_options.hpp>
#include <string>
#include "structures.h"

namespace po = boost::program_options;

// reads client program arguments and stores them
class ClientParameters {
public:
    ClientParameters(int argc, char *argv[]);

    std::string get_player_name();
    Address get_gui_address();
    Address get_server_address();
    uint16_t get_port();

private:
    void initialize_options_description();

    po::options_description opt_description{"Allowed options"};
    po::variables_map var_map;
};


#endif //ROBOTS_PARAMETERS_H
