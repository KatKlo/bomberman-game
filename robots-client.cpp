#include "parameters.h"
#include "utils.h"
#include <boost/asio.hpp>
#include "connections.h"

#include <iostream>

int main(int argc, char *argv[]) {
//    OutputBuffer o;
//    ClientMessages::Client_server_message_variant msg = ClientMessages::MoveMessage{std::string("👩🏼‍👩🏼‍👧🏼‍👦🏼🇵🇱")};
//    o.write_client_to_server_message(msg);
//    uint8_t *buf = (uint8_t *) o.get_buffer();
//    for (int i = 0; i < o.size(); i++) {
//        std::cout << (uint32_t) buf[i] << " ";
//    }
//
//    std::cout << "\n";

    ClientParameters p{argc, argv};

    boost::asio::io_context io_context;
    GUI_connection gui_server(io_context, p.get_gui_address(), p.get_port());


//    OutputBuffer o;
//    ClientMessages::Client_server_message_variant msg = ClientMessages::JoinMessage{std::string("hejka")};
//    o.write_client_to_server_message(msg);
//    gui_server.send(o);

    io_context.run();

    return 0;
}