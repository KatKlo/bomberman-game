#ifndef ROBOTS_CONNECTIONS_H
#define ROBOTS_CONNECTIONS_H

#include <boost/asio.hpp>
#include <deque>
#include "structures.h"
#include "buffer.h"
#include "game_state.h"
#include "parameters.h"
#include <unordered_map>

class GuiConnection;

class ClientConnection;

class ServerConnection;

// Class for handling receiving, handling and sending proper messages
class Client {
public:
    Client(boost::asio::io_context &io_context, ClientParameters &parameters);

    virtual ~Client();

    void handle_input_message(InputMessage::input_message_variant &&msg);

    void handle_server_message(ServerMessage::server_message_variant &&msg);

private:
    std::shared_ptr<GuiConnection> gui_connection_;
    std::shared_ptr<ServerConnection> server_connection_;
    ClientGameInfo gameInfo_;
};

class Server {
public:
    Server(boost::asio::io_context &io_context, ServerParameters &parameters);

    virtual ~Server();

    void handle_join_message(ClientMessage::Join &msg, std::shared_ptr<ClientConnection> client);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<ClientConnection>> client_connections_;
    std::unordered_map<player_id_t, std::shared_ptr<ClientConnection>> player_connections_;
    std::vector<ServerMessage::server_message_variant> messages_for_new_connection_;
    ServerMessage::Hello hello_message_;
    ServerGameInfo gameInfo_;
    boost::asio::steady_timer timer_;
    boost::asio::chrono::milliseconds timer_interval_;

    void do_accept();
    void send_message_to_all(ServerMessage::server_message_variant &msg);
    void send_and_save_message_to_all(ServerMessage::server_message_variant &msg);
    void play_game();
};

// Superclass for connections with gui and server
class Connection {
public:
    virtual void close() = 0;

protected:
    std::deque<OutputBuffer> write_msgs_;
    std::vector<uint8_t> buffer_;

    explicit Connection();
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
    Client &client_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    UdpInputBuffer read_msg_;

    void do_read_message();

    void do_write_message();
};

// Class for handling connection with server
class TCPConnection
        : public Connection {
public:
    TCPConnection(boost::asio::ip::tcp::socket socket);

    void close() override;

protected:
    boost::asio::ip::tcp::socket socket_;
    TcpInputBuffer read_msg_;

    void do_read_message();
    void do_write_message();
    virtual void handle_messages_in_bufor() = 0;
};

// Class for handling connection with server
class ServerConnection
        : public TCPConnection {
public:
    ServerConnection(boost::asio::io_context &io_context,
                     Address &&server_address,
                     Client &client);

    void send(ClientMessage::client_message_variant &msg);


private:
    Client &client_;

    void handle_messages_in_bufor() override;
};

class ClientConnection
        : public TCPConnection,
          public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(boost::asio::ip::tcp::socket socket, Server &server);

    void send(ServerMessage::server_message_variant &msg);

    ClientMessage::client_message_optional_variant get_latest_message();


private:
    Server &server_;
    ClientMessage::client_message_optional_variant last_message_;

    void handle_messages_in_bufor() override;
};

#endif //ROBOTS_CONNECTIONS_H
