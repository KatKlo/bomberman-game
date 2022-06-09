#ifndef ROBOTS_PARAMETERS_H
#define ROBOTS_PARAMETERS_H

#include <boost/program_options.hpp>
#include <string>

// Class for storing address given as parameter
struct Address {
    static constexpr uint16_t MIN_PORT = 0;
    static constexpr uint16_t MAX_PORT = 65535;

    std::string host;
    std::string port;

    friend std::istream &operator>>(std::istream &in, Address &adr);

    friend std::ostream &operator<<(std::ostream &out, const Address &adr);

    static bool validate_port_number(std::string &number_str);
};

class Parameters {
public:
    Parameters();

    // Result
    //  - TRUE if all parameters are correct and there's no help option
    //  - FALSE if there's help option
    //  - throws exception if any parameter is incorrect or there's help option
    bool read_program_arguments(int argc, char *argv[]);
protected:
    boost::program_options::options_description opt_description_;
    boost::program_options::variables_map var_map_;

private:
    virtual void initialize_options_description() = 0;
};

// Reads client program arguments and stores them
class ClientParameters : public Parameters {
public:
    ClientParameters();

    std::string get_player_name();
    Address get_gui_address();
    Address get_server_address();
    uint16_t get_port();

private:
    void initialize_options_description() override;
};

// Reads server program arguments and stores them
class ServerParameters : public Parameters {
public:
    ServerParameters();

    uint16_t get_bomb_timer();
    uint8_t get_players_count();
    uint64_t get_turn_duration();
    uint16_t get_explosion_radius();
    uint16_t get_initial_blocks();
    uint16_t get_game_length();
    std::string get_server_name();
    uint16_t get_port();
    uint32_t get_seed();
    uint16_t get_size_x();
    uint16_t get_size_y();

private:
    void initialize_options_description() override;
};

struct u8_t {
    uint8_t value;

    friend std::istream& operator>>(std::istream& in, u8_t& u8);
    friend std::ostream& operator<<(std::ostream& out, const u8_t& u8);
};

#endif //ROBOTS_PARAMETERS_H