#ifndef ROBOTS_SERVER_CONNECTIONS_H
#define ROBOTS_SERVER_CONNECTIONS_H

#include <boost/asio.hpp>
#include "../parameters.h"
#include "../structures.h"
#include "../game_managers/server_game_info.h"
#include "connections.h"
#include "../buffers/outgoing_buffer.h"
#include "../buffers/tcp_incoming_buffer.h"

class ClientConnection;

class Server {
public:
    Server(boost::asio::io_context &io_context, ServerParameters &parameters);

    virtual ~Server();

    void handle_join_message(ClientMessage::Join &msg, const std::shared_ptr<ClientConnection>& client);

    void disconnect_client(const std::shared_ptr<ClientConnection>& client);
private:
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_set<std::shared_ptr<ClientConnection>> client_connections_;
    std::unordered_map<player_id_t, std::shared_ptr<ClientConnection>> player_connections_;
    std::vector<ServerMessage::server_message_variant> messages_for_new_connection_;
    ServerMessage::Hello hello_message_;
    ServerGameInfo gameInfo_;
    boost::asio::steady_timer timer_;
    boost::asio::chrono::milliseconds timer_interval_;

    void do_accept();
    void send_message_to_all(ServerMessage::server_message_variant &&msg);
    void send_and_save_message_to_all(ServerMessage::server_message_variant &&msg);
    void play_game();
    void handle_turn();
    void make_turn();
};

class ClientConnection
        : public TCPConnection,
          public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(boost::asio::ip::tcp::socket socket, Server &server);

    void start();

    void send(ServerMessage::server_message_variant &msg);

    ClientMessage::client_message_optional_variant get_latest_message();

private:
    Server &server_;
    ClientMessage::client_message_optional_variant last_message_;

    void handle_messages_in_bufor() override;
    void handle_connection_error() override;
//    void do_write_message(const std::shared_ptr<ClientConnection> &&client);
};

#endif //ROBOTS_SERVER_CONNECTIONS_H
