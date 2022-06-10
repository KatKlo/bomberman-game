#include "server_connections.h"
#include "../logger.h"
#include <boost/bind/bind.hpp>

using tcp = boost::asio::ip::tcp;
using namespace std;

Server::Server(boost::asio::io_context &io_context,
               ServerParameters &parameters) : acceptor_(io_context, {boost::asio::ip::tcp::v6(), parameters.get_port()}),
                                               client_connections_(),
                                               player_connections_(),
                                               messages_for_new_connection_(),
                                               hello_message_(parameters),
                                               gameInfo_(parameters),
                                               timer_(io_context),
                                               timer_interval_(parameters.get_turn_duration()) {
    Logger::print_debug("server created - accepting clients on address ", acceptor_.local_endpoint());
    do_accept();
}

Server::~Server() {
    Logger::print_debug("closing server");

    for (auto &connection: client_connections_) {
        connection->close();
    }

    acceptor_.close();
}

void Server::do_accept() {
    acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    auto new_client = make_shared<ClientConnection>(move(socket), *this);
                    ServerMessage::server_message first_msg = hello_message_;

                    new_client->start();
                    new_client->send(first_msg);
                    for (auto &msg: messages_for_new_connection_) {
                        new_client->send(msg);
                    }

                    client_connections_.emplace(new_client);

                    Logger::print_debug("client ", new_client->get_address(), " added to connected clients");
                }

                do_accept();
            });
}

void Server::send_message_to_all(ServerMessage::server_message &&msg) {
    for (auto &connection: client_connections_) {
        connection->send(msg);
    }
}

void Server::send_and_save_message_to_all(ServerMessage::server_message &&msg) {
    messages_for_new_connection_.emplace_back(msg);
    send_message_to_all(move(msg));
}

void Server::handle_join_message(ClientMessage::Join &msg, const shared_ptr<ClientConnection> &client) {
    optional<ServerMessage::AcceptedPlayer> possible_msg = gameInfo_.handle_client_join_message(msg, client->get_address());

    if (possible_msg.has_value()) {
        player_connections_.emplace(possible_msg.value().id, client);
        send_and_save_message_to_all(move(possible_msg.value()));

        if (gameInfo_.is_enough_players()) {
            // we want to start immediately but make it async
            timer_.async_wait(boost::bind(&Server::play_game, this));
        }
    }
}

void Server::handle_turn() {
    unordered_map<player_id_t, ClientMessage::client_message> messages_to_handle;

    for (auto &player: player_connections_) {
        ClientMessage::client_message_optional player_msg = player.second->get_latest_message();

        if (player_msg.has_value()) {
            messages_to_handle.emplace(player.first, move(player_msg.value()));
        }
    }

    send_and_save_message_to_all(gameInfo_.handle_turn(messages_to_handle));

    if (gameInfo_.is_end_of_game()) {
        messages_for_new_connection_.clear();
        player_connections_.clear();

        send_message_to_all(gameInfo_.end_game());
    } else {
        timer_.expires_after(timer_interval_);
        timer_.async_wait(boost::bind(&Server::handle_turn, this));
    }
}

void Server::play_game() {
    messages_for_new_connection_.clear();
    ServerGameInfo::start_game_messages initial_msgs = gameInfo_.start_game();
    send_and_save_message_to_all(initial_msgs.first);
    send_and_save_message_to_all(initial_msgs.second);

    // clean messages sent before game started
    for (auto &player: player_connections_) {
        ClientMessage::client_message_optional player_msg = player.second->get_latest_message();
    }

    timer_.expires_after(timer_interval_);
    timer_.async_wait(boost::bind(&Server::handle_turn, this));
}

void Server::disconnect_client(const shared_ptr<ClientConnection> &client) {
    client_connections_.erase(client);
}

ClientConnection::ClientConnection(boost::asio::ip::tcp::socket socket,
                                   Server &server) : TCPConnection(move(socket)),
                                                     server_(server),
                                                     last_message_() {
    set_proper_address();
}

void ClientConnection::start() {
    do_read_message();
}

void ClientConnection::send(ServerMessage::server_message &msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.emplace_back(msg);

    if (!write_in_progress) {
        do_write_message();
    }
}

ClientMessage::client_message_optional ClientConnection::get_latest_message() {
    ClientMessage::client_message_optional result = last_message_;
    last_message_ = nullopt;

    return result;
}

void ClientConnection::handle_messages_in_bufor() {
    bool is_sth_to_read_in_buffer = true;
    while (is_sth_to_read_in_buffer) {
        try {
            auto msg = read_msg_.read_client_message();

            if (msg.index() == ClientMessage::JOIN) {
                server_.handle_join_message(get<ClientMessage::Join>(msg), shared_from_this());
            } else {
                last_message_ = msg;
            }
        } catch (length_error &e) { // invalid argument should break whole program
            is_sth_to_read_in_buffer = false;
        }
    }
}


void ClientConnection::handle_connection_error() {
    server_.disconnect_client(shared_from_this());
}

