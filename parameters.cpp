#include "parameters.h"
#include "logger.h"

namespace po = boost::program_options;

Parameters::Parameters() : opt_description_("options"), var_map_() {}


bool Parameters::read_program_arguments(int argc, char **argv) {
    try {
        po::store(po::parse_command_line(argc, argv, opt_description_), var_map_);
        po::notify(var_map_);

        // we want to see a help information even if
        // all parameters are correct, but there's a help option
        if (var_map_.count("help")) {
            throw std::logic_error("help in options");
        }

        return true;
    }
    catch (std::exception &e) {
        if (var_map_.count("help") == 0) {
            throw;
        }

        Logger::print_info("Usage: ", argv[0], " with ", opt_description_);
        return false;
    }
}

ClientParameters::ClientParameters() : Parameters() {
    ClientParameters::initialize_options_description();
}

std::string ClientParameters::get_player_name() {
    return var_map_["player-name"].as<std::string>();
}

uint16_t ClientParameters::get_port() {
    return var_map_["port"].as<uint16_t>();
}

Address ClientParameters::get_gui_address() {
    return var_map_["gui-address"].as<Address>();
}

Address ClientParameters::get_server_address() {
    return var_map_["server-address"].as<Address>();
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

    opt_description_.add(required_description).add(optional_description);
}


ServerParameters::ServerParameters() : Parameters() {
    ServerParameters::initialize_options_description();
}

uint16_t ServerParameters::get_bomb_timer(){
    return var_map_["bomb-timer"].as<uint16_t>();
}

uint8_t ServerParameters::get_players_count() {
    return var_map_["players-count"].as<uint8_t>();
}

uint64_t ServerParameters::get_turn_duration() {
    return var_map_["turn-duration"].as<uint64_t>();
}

uint16_t ServerParameters::get_explosion_radius() {
    return var_map_["explosion-radius"].as<uint16_t>();
}

uint16_t ServerParameters::get_initial_blocks() {
    return var_map_["initial-blocks"].as<uint16_t>();
}

uint16_t ServerParameters::get_game_length() {
    return var_map_["game-length"].as<uint16_t>();
}

std::string ServerParameters::get_server_name() {
    return var_map_["server-name"].as<std::string>();
}

uint16_t ServerParameters::get_port() {
    return var_map_["port"].as<uint16_t>();
}

uint32_t ServerParameters::get_seed() {
    return var_map_["seed"].as<uint32_t>();
}

uint16_t ServerParameters::get_size_x() {
    return var_map_["size-x"].as<uint16_t>();
}

uint16_t ServerParameters::get_size_y() {
    return var_map_["size-y"].as<uint16_t>();
}

void ServerParameters::initialize_options_description() {
    po::options_description required_description("Required options");
    required_description.add_options()
            ("bomb-timer,b", po::value<uint16_t>()->required(), "set bomb timer")
            ("players-count,c", po::value<uint8_t>()->required(), "set players count to start game")
            ("turn-duration,d", po::value<uint64_t>()->required(), "set turn duration")
            ("explosion-radius,e", po::value<uint16_t>()->required(), "set explosion radius")
            ("initial-blocks,k", po::value<uint16_t>()->required(), "set initial blocks count")
            ("game-length,l", po::value<uint16_t>()->required(), "set game length")
            ("server-name,n", po::value<std::string>()->required(), "set server name")
            ("port,p", po::value<uint16_t>()->required(), "set port number for receiving clients messages")
            ("seed,s", po::value<uint32_t>()->required(), "set seed for random generator")
            ("size-x,x", po::value<uint16_t>()->required(), "set map size x")
            ("size-y,y", po::value<uint16_t>()->required(), "set map size y");

    po::options_description optional_description("Optional options");
    optional_description.add_options()
            ("help,h", "print help information");

    opt_description_.add(required_description).add(optional_description);
}

bool Address::validate_port_number(std::string &number_str) {
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

    if (!Address::validate_port_number(address.port)) {
        throw std::invalid_argument(std::string("wrong port number in address: ") + token);
    }

    return in;
}

std::ostream &operator<<(std::ostream &out, const Address &adr) {
    return out << adr.host << ":" << adr.port;
}
