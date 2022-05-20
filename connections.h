#ifndef ROBOTS_CONNECTIONS_H
#define ROBOTS_CONNECTIONS_H

#include <boost/asio.hpp>
#include "structures.h"
#include "buffer.h"
#include "game_state.h"
#include "parameters.h"

class GUIConnection;
class ServerConnection;

class Client {
public:
    Client(boost::asio::io_context &io_context, ClientParameters &parameters);

    void handle_GUI_message(GUIMessages::GUI_message_variant &msg);
    void handle_server_message(ServerMessage::Server_message_variant &msg);

private:
    std::shared_ptr<GUIConnection> gui_connection;
    std::shared_ptr<ServerConnection> server_connection;
    std::optional<GameInfo> gameInfo;
    std::string name;
};

class GUIConnection {
public:
    GUIConnection(boost::asio::io_context &io_context, Address &&gui_address, uint16_t port, Client &client);

//    void close_connection();
    void send(ClientMessages::Client_GUI_message_variant &msg);

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
    void send(ClientMessages::Client_server_message_variant &msg);

private:
    Client &client_;
    boost::asio::ip::tcp::socket socket;

    TcpInputBuffer read_msg;
    OutputBuffer write_msg;
    std::vector<uint8_t> buffer_;

    void do_read_message();
};

#endif //ROBOTS_CONNECTIONS_H
