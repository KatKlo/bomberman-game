#ifndef ROBOTS_CONNECTIONS_H
#define ROBOTS_CONNECTIONS_H

#include <boost/asio.hpp>
#include "structures.h"
#include "buffer.h"
//
//class GUI_connection;
//
//class Client {
//private:
//    GUI_connection gui_connection;
//};

class GUI_connection {
public:
    GUI_connection(boost::asio::io_context &io_context, Address &&gui_address, uint16_t port);
//    void close_connection();
    void send(ClientMessages::Client_GUI_message_variant &msg);

private:
    boost::asio::ip::udp::socket socket;
    InputBuffer read_msg;
    OutputBuffer::output_buffer_queue write_msgs;
    boost::asio::ip::udp::endpoint remote_endpoint_;

    void do_read_message();
    void do_write();
};

#endif //ROBOTS_CONNECTIONS_H
