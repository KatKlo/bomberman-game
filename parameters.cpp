#include "parameters.h"
#include "logger.h"

namespace po = boost::program_options;

ClientParameters::ClientParameters() : opt_description("options"), var_map() {
    initialize_options_description();
}

bool ClientParameters::read_program_arguments(int argc, char **argv) {
    try {
        po::store(po::parse_command_line(argc, argv, opt_description), var_map);
        po::notify(var_map);

        // we want to see a help information even if
        // all parameters are correct, but there's a help option
        if (var_map.count("help")) {
            throw std::logic_error("help in options");
        }

        return true;
    }
    catch (std::exception &e) {
        if (var_map.count("help") == 0) {
            Logger::print_error(e.what());
        }

        Logger::print_info("Usage: ", argv[0], " with ", opt_description);
        return false;
    }
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

void ClientParameters::initialize_options_description() {
    po::options_description required_description("Required options");
    required_description.add_options()
            ("gui-address,d", po::value<Address>()->required(),
             "set gui address, arg as (nazwa hosta):(port) or (IPv4):(port) or (IPv6):(port)")
            ("player-name,n", po::value<std::string>()->required(), "set player's name")
            ("port,p", po::value<uint16_t>()->required(), "set port number for receiving gui messages")
            ("server-address,s", po::value<Address>()->required(),
             "set server address, arg as (nazwa hosta):(port) or (IPv4):(port) or (IPv6):(port)");

    po::options_description optional_description("Optional options");
    optional_description.add_options()
            ("help,h", "print help information");

    opt_description.add(required_description).add(optional_description);
}

static bool validate_port_number(std::string &number_str) {
    errno = 0;
    char *end;
    unsigned long number = strtoul(number_str.c_str(), &end, 10);

    if (*end != '\0' || errno != 0 || number > Address::MAX_PORT || number < Address::MIN_PORT) {
        return false;
    }

    return true;
}

std::istream &operator>>(std::istream &in, Address &address) {
    std::string token;
    in >> token;

    size_t delimiter_index = token.find_last_of(':');
    if (delimiter_index == std::string::npos || delimiter_index == token.size() - 1) {
        throw std::invalid_argument(std::string("no port number in address: ") + token);
    }

    address.port = token.substr(delimiter_index + 1, token.size() - 1 - delimiter_index);
    address.host = token.substr(0, delimiter_index);

    if (!validate_port_number(address.port)) {
        throw std::invalid_argument(std::string("wrong port number in address: ") + token);
    }

    return in;
}

std::ostream &operator<<(std::ostream &out, const Address &adr) {
    return out << adr.host << ":" << adr.port;
}