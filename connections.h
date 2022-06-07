#ifndef ROBOTS_CONNECTIONS_H
#define ROBOTS_CONNECTIONS_H

#include <boost/asio.hpp>
#include <deque>
#include "structures.h"
#include "buffer.h"
#include "game_state.h"
#include "parameters.h"

class GuiConnection;

class ServerConnection;

// Class for handling receiving, handling and sending proper messages
class Client {
public:
    Client(boost::asio::io_context &io_context, ClientParameters &parameters);

    virtual ~Client();

    void handle_input_message(InputMessage::input_message_variant &&msg);
    void handle_server_message(ServerMessage::server_message_variant &&msg);

    void close_connections();


private:
    std::shared_ptr<GuiConnection> gui_connection_;
    std::shared_ptr<ServerConnection> server_connection_;
    ClientGameInfo gameInfo_;
};

// Superclass for connections with gui and server
class Connection {
public:
    virtual void close() = 0;

protected:
    Client &client_;
    std::deque<OutputBuffer> write_msgs_;
    std::vector<uint8_t> buffer_;

    explicit Connection(Client &client);
};

// Class for handling connection with gui
class GuiConnection
        : public Connection {
public:
    GuiConnection(boost::asio::io_context &io_context,
                  Address &&gui_address,
                  uint16_t port,
                  Client &client);

    void close() override;

    void send(DrawMessage::draw_message_variant &msg);

private:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    UdpInputBuffer read_msg_;

    void do_read_message();
    void do_write_message();
};

// Class for handling connection with server
class ServerConnection
        : public Connection {
public:
    ServerConnection(boost::asio::io_context &io_context,
                     Address &&server_address,
                     Client &client);

    void close() override;

    void send(ClientMessage::client_message_variant &msg);


private:
    boost::asio::ip::tcp::socket socket_;
    TcpInputBuffer read_msg_;

    void do_read_message();
    void do_write_message();
};

#endif //ROBOTS_CONNECTIONS_H
