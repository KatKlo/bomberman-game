#ifndef ROBOTS_CLIENT_CONNECTIONS_H
#define ROBOTS_CLIENT_CONNECTIONS_H

#include <boost/asio.hpp>
#include "../parameters.h"
#include "../structures.h"
#include "../game_managers/client_game_info.h"
#include "../buffers/udp_incoming_buffer.h"
#include "connections.h"

class GuiConnection;
class ServerConnection;

// Class for handling receiving, handling and sending proper messages
class Client {
public:
    Client(boost::asio::io_context &io_context, ClientParameters &parameters);

    virtual ~Client();

    void handle_input_message(InputMessage::input_message &&msg);
    void handle_server_message(ServerMessage::server_message &&msg);

private:
    std::shared_ptr<GuiConnection> gui_connection_;
    std::shared_ptr<ServerConnection> server_connection_;
    ClientGameInfo gameInfo_;
};

// Class for handling connection with gui
class GuiConnection : public Connection {
public:
    GuiConnection(boost::asio::io_context &io_context, Address &&gui_address,
                  uint16_t port, Client &client);

    void close() override;

    void send(DrawMessage::draw_message &msg);

private:
    Client &client_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    UdpIncomingBuffer read_msg_;

    void do_read_message();
    void do_write_message();
};

// Class for handling connection with server
class ServerConnection : public TCPConnection {
public:
    ServerConnection(boost::asio::io_context &io_context,
                     Address &&server_address, Client &client);

    void send(ClientMessage::client_message &msg);

private:
    Client &client_;

    void handle_messages_in_bufor() override;
    void handle_connection_error() override;
};


#endif //ROBOTS_CLIENT_CONNECTIONS_H
