#ifndef ROBOTS_SERVER_CONNECTIONS_H
#define ROBOTS_SERVER_CONNECTIONS_H

#include "../parameters.h"
#include "../structures.h"
#include "../game_managers/server_game_info.h"
#include "connections.h"
#include <boost/asio.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ClientConnection;

class Server {
public:
    Server(boost::asio::io_context &io_context, ServerParameters &parameters);

    virtual ~Server();

    void handle_join_message(ClientMessage::Join &msg, const std::shared_ptr<ClientConnection> &client);

    void disconnect_client(const std::shared_ptr<ClientConnection> &client);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_set<std::shared_ptr<ClientConnection>> client_connections_;
    std::unordered_map<player_id_t, std::shared_ptr<ClientConnection>> player_connections_;
    std::vector<ServerMessage::server_message> messages_for_new_connection_;
    ServerMessage::Hello hello_message_;
    ServerGameInfo gameInfo_;
    boost::asio::steady_timer timer_;
    boost::asio::chrono::milliseconds timer_interval_;

    void do_accept();

    void send_message_to_all(ServerMessage::server_message &&msg);
    void send_and_save_message_to_all(ServerMessage::server_message &&msg);

    void play_game();
    void handle_turn();
};

class ClientConnection :
        public TCPConnection,
        public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(boost::asio::ip::tcp::socket socket, Server &server);

    void start();

    void send(ServerMessage::server_message &msg);

    ClientMessage::client_message_optional get_latest_message();

private:
    Server &server_;
    ClientMessage::client_message_optional last_message_;

    void handle_messages_in_bufor() override;
    void handle_connection_error() override;
};

#endif //ROBOTS_SERVER_CONNECTIONS_H
