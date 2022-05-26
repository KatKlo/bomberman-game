#ifndef ROBOTS_PARAMETERS_H
#define ROBOTS_PARAMETERS_H

#include <boost/program_options.hpp>
#include <string>

// class for storing address given as parameter
struct Address {
    static constexpr uint16_t MIN_PORT = 0;
    static constexpr uint16_t MAX_PORT = 65535;

    std::string host;
    std::string port;

    friend std::istream &operator>>(std::istream &in, Address &address);

    friend std::ostream &operator<<(std::ostream &out, const Address &adr);
};

// reads client program arguments and stores them
class ClientParameters {
public:
    ClientParameters();

    // returns:
    //  - TRUE if all parameters are correct and there's no help option
    //  - FALSE if any parameter is incorrect or there's help option
    bool read_program_arguments(int argc, char *argv[]);

    std::string get_player_name();
    Address get_gui_address();
    Address get_server_address();
    uint16_t get_port();

private:
    void initialize_options_description();

    boost::program_options::options_description opt_description_;
    boost::program_options::variables_map var_map_;
};

#endif //ROBOTS_PARAMETERS_H