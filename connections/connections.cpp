#include "connections.h"
#include "../logger.h"

using namespace std;

Connection::Connection() : write_msgs_(), buffer_(Buffer::MAX_PACKET_LENGTH) {}

void TCPConnection::do_read_message() {
    socket_.async_read_some(
            boost::asio::buffer(&buffer_[0], Buffer::MAX_PACKET_LENGTH),
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    Logger::print_debug("read message from ", address_, " - ", length, " bytes");

                    read_msg_.add_packet(buffer_, length);
                    
                    handle_messages_in_bufor();
                    do_read_message();
                } else {
                    handle_connection_error();
                }
            });
}

void TCPConnection::close() {
    Logger::print_debug("closing tcp connection with ", address_);
    socket_.close();
}

void TCPConnection::do_write_message() {
    socket_.async_send(
            boost::asio::buffer(write_msgs_.front().get_buffer(), write_msgs_.front().size()),
            [this](boost::system::error_code ec, size_t) {
                if (!ec) {
                    Logger::print_debug("send message to ", address_, " - ", write_msgs_.front().size(), 
                                        " bytes: ", write_msgs_.front());
                    
                    write_msgs_.pop_front();

                    if (!write_msgs_.empty()) {
                        do_write_message();
                    }
                } else {
                    handle_connection_error();
                }
            });
}

TCPConnection::TCPConnection(boost::asio::ip::tcp::socket socket) : Connection(),
                                                                    socket_(move(socket)),
                                                                    read_msg_(),
                                                                    address_() {}

string TCPConnection::get_address() {
    return address_;
}

void TCPConnection::set_proper_address() {
    stringstream s;
    s << socket_.remote_endpoint();
    s >> address_;
}
