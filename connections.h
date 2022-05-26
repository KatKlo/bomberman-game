#ifndef ROBOTS_CONNECTIONS_H
#define ROBOTS_CONNECTIONS_H

#include <boost/asio.hpp>
#include "structures.h"
#include "buffer.h"
#include "game_state.h"
#include "parameters.h"
#include <mutex>

class GUIConnection;

class ServerConnection;

class Client {
public:
    Client(boost::asio::io_context &io_context, ClientParameters &parameters);

    void handle_input_message(InputMessage::input_message_variant &msg);

    void handle_server_message(ServerMessage::server_message_variant &msg);

    void close_connections();

private:
    std::shared_ptr<GUIConnection> gui_connection;
    std::shared_ptr<ServerConnection> server_connection;
    GameInfo gameInfo;
    std::string name;
    std::mutex guard;
};

class GUIConnection {
public:
    GUIConnection(boost::asio::io_context &io_context, Address &&gui_address, uint16_t port, Client &client);

//    void close_connection();
    void send(DrawMessage::draw_message_variant &msg);

    void close();

private:
    Client &client_;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint remote_endpoint_;

    UdpInputBuffer read_msg;
    OutputBuffer write_msg;
    std::vector<uint8_t> buffer_;

    void do_read_message();
};

class ServerConnection {
public:
    ServerConnection(boost::asio::io_context &io_context, Address &&server_address, Client &client);

//    void close_connection();
    void send(ClientMessage::client_message_variant &msg);

    void close();

private:
    Client &client_;
    boost::asio::ip::tcp::socket socket;

    TcpInputBuffer read_msg;
    OutputBuffer write_msg;
    std::vector<uint8_t> buffer_;

    void do_read_message();
};

#endif //ROBOTS_CONNECTIONS_H
