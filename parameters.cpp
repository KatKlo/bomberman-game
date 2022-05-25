#include "parameters.h"
#include "structures.h"
#include "utils.h"
#include <iostream>

ClientParameters::ClientParameters(int argc, char *argv[]) : opt_description("Allowed options"), var_map() {
    try {
        initialize_options_description();
        po::store(po::parse_command_line(argc, argv, opt_description), var_map);
        po::notify(var_map);

//        if (var_map.count("help")) {
//            Logger::print_info("opt_description");
//            exit(0);
//        } else if (var_map.size() != 4) {
//            Logger::print_error("to little arguments");
//            exit(1);
//        }
    }
    catch (std::exception &e) {
        if (var_map.count("help")) {
            Logger::print_info("Usage:\n", opt_description);
        } else {
            Logger::print_error(e.what());
        }

        throw e;
    }
}

void ClientParameters::initialize_options_description() {
    opt_description.add_options()
            ("gui-address,d", po::value<Address>()->required(),
             "set gui address, arg as (nazwa hosta):(port) or (IPv4):(port) or (IPv6):(port)")
            ("help,h", "print help information")
            ("player-name,n", po::value<std::string>()->required(), "set player's name")
            ("port,p", po::value<uint16_t>()->required(), "set port number for receiving gui messages")
            ("server-address,s", po::value<Address>()->required(),
             "set server address, arg as (nazwa hosta):(port) or (IPv4):(port) or (IPv6):(port)");
}

std::string ClientParameters::get_player_name() {
    return var_map["player-name"].as<std::string>();
}

uint16_t ClientParameters::get_port() {
    return var_map["port"].as<uint16_t>();
}

Address ClientParameters::get_gui_address() {
    return var_map["gui-address"].as<Address>();
}

Address ClientParameters::get_server_address() {
    return var_map["server-address"].as<Address>();
}
