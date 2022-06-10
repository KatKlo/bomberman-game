#ifndef ROBOTS_CONNECTIONS_H
#define ROBOTS_CONNECTIONS_H

#include "../buffers/outgoing_buffer.h"
#include "../buffers/tcp_incoming_buffer.h"
#include <boost/asio.hpp>
#include <deque>

// Superclass for connections with gui and server
class Connection {
public:
    virtual void close() = 0;

protected:
    std::deque<OutgoingBuffer> write_msgs_;
    std::vector<uint8_t> buffer_;

    explicit Connection();
};

// Class for handling connection with server
class TCPConnection : public Connection {
public:
    explicit TCPConnection(boost::asio::ip::tcp::socket socket);

    void close() override;

    std::string get_address();
    void set_proper_address();

protected:
    boost::asio::ip::tcp::socket socket_;
    TcpIncomingBuffer read_msg_;
    std::string address_;

    void do_read_message();
    void do_write_message();

    virtual void handle_messages_in_bufor() = 0;
    virtual void handle_connection_error() = 0;
};

#endif //ROBOTS_CONNECTIONS_H