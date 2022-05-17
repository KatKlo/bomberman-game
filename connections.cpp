#include "connections.h"
#include <boost/asio.hpp>
#include "structures.h"
#include "buffer.h"
#include "utils.h"

using boost::asio::ip::udp;

GUI_connection::GUI_connection(boost::asio::io_context &io_context, Address &&gui_address, uint16_t port)
        : socket(io_context, udp::endpoint(udp::v4(), port)) {
    Logger::print_debug("connecting to GUI server (", gui_address, ") on port ", port);

//    boost::asio::ip::udp::resolver resolver(io_context);
//    auto endpoints = resolver.resolve(gui_address.host, gui_address.port);
//    boost::asio::connect(socket, endpoints);

//    Logger::print_debug("GUI server connected");

    Logger::print_debug("starting GUI connection");
    do_read_message();
}

//void GUI_connection::close_connection() {
//    Logger::print_debug("Closing GUI server connection");
//    socket.shutdown(udp::socket::shutdown_both);
//    socket.close();
//}

void GUI_connection::send(ClientMessages::Client_GUI_message_variant &msg) {
    Logger::print_debug("starting sending");
    bool write_in_progress = !write_msgs.empty();
    OutputBuffer o;
    o.write_client_to_GUI_message(msg);
    write_msgs.push_back(o);
    if (!write_in_progress) {
        do_write();
    }
}

void GUI_connection::do_read_message() {
    socket.async_receive_from(
            boost::asio::buffer(read_msg.get_buffer(), Buffer::MAX_UDP_LENGTH),
            remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t length) {
                Logger::print_debug("do read");
                if (!ec) {
                    read_msg.set_size(length);
                    auto sth = read_msg.read_GUI_message();
                    // TODO realizacja wiadomości
                    Logger::print_debug("Received message: ", read_msg.get_buffer());
                    do_read_message();
                } else {
                    // TODO realizacja erroru
                }
            });
}

void GUI_connection::do_write() {
    socket.async_send_to(
            boost::asio::buffer(write_msgs.front().get_buffer(), write_msgs.front().size()),
            remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t /*length*/) {
                Logger::print_debug("do write");
                if (!ec) {
                    write_msgs.pop_front();
                    if (!write_msgs.empty()) {
                        do_write();
                    }
                } else {
                    // TODO realizacja erroru
                }
            });

}
