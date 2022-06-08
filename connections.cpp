#include "connections.h"
#include "logger.h"

using udp = boost::asio::ip::udp;
using tcp = boost::asio::ip::tcp;

Client::Client(boost::asio::io_context &io_context,
               ClientParameters &parameters) : gui_connection_(),
                                               server_connection_(),
                                               gameInfo_(parameters.get_player_name()) {

    gui_connection_ = std::make_shared<GuiConnection>(io_context,
                                                      parameters.get_gui_address(),
                                                      parameters.get_port(),
                                                      *this);

    server_connection_ = std::make_shared<ServerConnection>(io_context,
                                                            parameters.get_server_address(),
                                                            *this);

}

void Client::handle_input_message(InputMessage::input_message_variant &&msg) {
    ClientMessage::client_message_optional_variant new_msg = gameInfo_.handle_GUI_message(msg);
    if (new_msg.has_value()) {
        server_connection_->send(new_msg.value());
    }
}

void Client::handle_server_message(ServerMessage::server_message_variant &&msg) {
    DrawMessage::draw_message_optional_variant new_msg = gameInfo_.handle_server_message(msg);
    if (new_msg.has_value()) {
        gui_connection_->send(new_msg.value());
    }
}

Client::~Client() {
    Logger::print_debug("closing client connections");
    gui_connection_->close();
    server_connection_->close();
}

Connection::Connection() : write_msgs_(),
                           buffer_(Buffer::MAX_PACKET_LENGTH) {}

GuiConnection::GuiConnection(boost::asio::io_context &io_context,
                             Address &&gui_address,
                             uint16_t port,
                             Client &client) : Connection(),
                                               client_(client),
                                               socket_(io_context, udp::endpoint(udp::v6(), port)),
                                               remote_endpoint_(),
                                               read_msg_() {
    Logger::print_debug("creating gui connection");

    udp::resolver resolver(io_context);
    remote_endpoint_ = *resolver.resolve(gui_address.host, gui_address.port);

    Logger::print_debug("gui connected - sending on address ", remote_endpoint_,
                        " receiving on port ", socket_.local_endpoint());

    do_read_message();
}

void GuiConnection::close() {
    socket_.close();
    Logger::print_debug("gui connection closed");
}

void GuiConnection::send(DrawMessage::draw_message_variant &msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.emplace_back(msg);

    if (!write_in_progress) {
        do_write_message();
    }
}

void GuiConnection::do_read_message() {
    socket_.async_receive_from(
            boost::asio::buffer(&buffer_[0], Buffer::MAX_PACKET_LENGTH),
            remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t length) {
                if (ec == boost::asio::error::operation_aborted) { // closed socket_
                    return;
                }

                Logger::print_debug("read message from gui - ", length, " bytes");

                if (!ec) {
                    read_msg_.add_packet(buffer_, length);

                    try {
                        client_.handle_input_message(read_msg_.read_input_message());
                    } catch (std::exception &e) { // it's udp - ignore incorrect messages
                        Logger::print_debug("bad message from gui - ", e.what());
                    }
                } else {
                    Logger::print_debug("error in reading from gui");
                }

                do_read_message();
            }

    );
}

void GuiConnection::do_write_message() {
    socket_.async_send_to(
            boost::asio::buffer(write_msgs_.front().get_buffer(), write_msgs_.front().size()),
            remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t) {
                if (ec == boost::asio::error::operation_aborted) { // closed socket_
                    return;
                }

                Logger::print_debug("send message to gui - ", write_msgs_.front().size(),
                                    " bytes: ", write_msgs_.front());
                write_msgs_.pop_front();

                if (!ec) {
                    if (!write_msgs_.empty()) {
                        do_write_message();
                    }
                } else { // it's udp - ignore problems with sending
                    Logger::print_debug("error in writing to gui");
                }
            });
}

ServerConnection::ServerConnection(boost::asio::io_context &io_context,
                                   Address &&server_address,
                                   Client &client) : TCPConnection(boost::asio::ip::tcp::socket(io_context)),
                                                     client_(client) {
    Logger::print_debug("creating server connection");

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(server_address.host, server_address.port);
    boost::asio::connect(socket_, endpoints);
    socket_.set_option(tcp::no_delay(true));

    Logger::print_debug("server connected - sending on address ", socket_.remote_endpoint(),
                        " receiving on address ", socket_.local_endpoint());

    do_read_message();
}

void ServerConnection::handle_messages_in_bufor() {
    bool is_sth_to_read_in_buffer = true;
    while (is_sth_to_read_in_buffer) {
        try {
            client_.handle_server_message(read_msg_.read_server_message());
        } catch (std::length_error &e) { // invalid argument should break whole program
            is_sth_to_read_in_buffer = false;
        }
    }
}

void TCPConnection::do_read_message() {
    socket_.async_read_some(
            boost::asio::buffer(&buffer_[0], Buffer::MAX_PACKET_LENGTH),
            [this](boost::system::error_code ec, std::size_t length) {
                if (ec == boost::asio::error::operation_aborted) { // closed socket_
                    return;
                }

                Logger::print_debug("read message from ", socket_.remote_endpoint(), " - ", length, " bytes");

                if (!ec) {
                    read_msg_.add_packet(buffer_, length);
                    handle_messages_in_bufor();
                    do_read_message();
                } else {
                    throw std::domain_error("error in reading from " + socket_.remote_endpoint().address().to_string());
                }
            });
}

void ServerConnection::send(ClientMessage::client_message_variant &msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.emplace_back(msg);

    if (!write_in_progress) {
        do_write_message();
    }
}

void TCPConnection::close() {
    Logger::print_debug("closing tcp connection between", socket_.local_endpoint(), " and ", socket_.remote_endpoint());
    socket_.close();
}

void TCPConnection::do_write_message() {
    socket_.async_send(
            boost::asio::buffer(write_msgs_.front().get_buffer(), write_msgs_.front().size()),
            [this](boost::system::error_code ec, std::size_t) {
                if (ec == boost::asio::error::operation_aborted) { // closed socket_
                    return;
                }

                Logger::print_debug("send message to ", socket_.remote_endpoint(), " - ", write_msgs_.front().size(),
                                    " bytes: ", write_msgs_.front());
                write_msgs_.pop_front();

                if (!ec) {
                    if (!write_msgs_.empty()) {
                        do_write_message();
                    }
                } else {
                    throw std::domain_error("error in sending to " + socket_.remote_endpoint().address().to_string());
                }
            });
}

TCPConnection::TCPConnection(boost::asio::ip::tcp::socket socket) : Connection(),
                                                                    socket_(std::move(socket)),
                                                                    read_msg_() {}

ClientConnection::ClientConnection(boost::asio::ip::tcp::socket socket, Server &server) : TCPConnection(
        std::move(socket)),
                                                                                          server_(server),
                                                                                          last_message_() {}


void ClientConnection::send(ServerMessage::server_message_variant &msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.emplace_back(msg);

    if (!write_in_progress) {
        do_write_message();
    }
}

ClientMessage::client_message_optional_variant ClientConnection::get_latest_message() {
    return last_message_;
}

void ClientConnection::handle_messages_in_bufor() {
    bool is_sth_to_read_in_buffer = true;
    while (is_sth_to_read_in_buffer) {
        try {
            auto msg = read_msg_.read_client_message();
            if (msg.index() == ClientMessage::JOIN) {
                server_.handle_join_message(std::get<ClientMessage::Join>(msg), shared_from_this());
            } else {
                last_message_ = msg;
            }
        } catch (std::length_error &e) { // invalid argument should break whole program
            is_sth_to_read_in_buffer = false;
        }
    }
}

std::string ClientConnection::get_address() {
    return socket_.remote_endpoint().address().to_string();
}

Server::Server(boost::asio::io_context &io_context, ServerParameters &parameters) : acceptor_(io_context, {boost::asio::ip::tcp::v6(), parameters.get_port()}),
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
    for (auto &connection : client_connections_) {
        connection->close();
    }
    acceptor_.close();
}

void Server::do_accept() {
    acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    auto sth = std::make_shared<ClientConnection>(std::move(socket), *this);
                    ServerMessage::server_message_variant sth2 = hello_message_;
                    sth->send(sth2);
                    for (auto &msg : messages_for_new_connection_) {
                        sth->send(msg);
                    }
                    client_connections_.emplace_back(std::move(sth));
                }

                do_accept();
            });
}

void Server::send_message_to_all(ServerMessage::server_message_variant &msg) {
    for (auto &connection : client_connections_) {
        connection->send(msg);
    }
}

void Server::send_and_save_message_to_all(ServerMessage::server_message_variant &msg) {
    send_message_to_all(msg);
    messages_for_new_connection_.emplace_back(std::move(msg));
}

void Server::handle_join_message(ClientMessage::Join &msg, std::shared_ptr<ClientConnection> client) {
    std::optional<ServerMessage::AcceptedPlayer> sth = gameInfo_.handle_client_join_message(msg, client->get_address());
    if (sth.has_value()) {
        player_connections_.emplace(sth.value().id, std::move(client));
        ServerMessage::server_message_variant sth2 = sth.value();
        send_and_save_message_to_all(sth2);

        if (gameInfo_.is_enough_players()) {
            play_game();
        }
    }
}

void Server::play_game() {
    messages_for_new_connection_.clear();
    ServerMessage::server_message_variant sth = gameInfo_.start_game();
    send_and_save_message_to_all(sth);

    std::unordered_map<player_id_t, ClientMessage::client_message_variant> messages_to_handle;

    sth = gameInfo_.handle_turn(messages_to_handle);
    send_and_save_message_to_all(sth);

    while (!gameInfo_.is_end_of_game()) {
        timer_.expires_from_now(timer_interval_);
        timer_.wait();

        for (auto &player : player_connections_) {
            ClientMessage::client_message_optional_variant sth2 = player.second->get_latest_message();
            if (sth2.has_value()) {
                messages_to_handle.emplace(player.first, std::move(sth2.value()));
            }
        }

        sth = gameInfo_.handle_turn(messages_to_handle);
        send_message_to_all(sth);
        messages_to_handle.clear();
    }

    sth = gameInfo_.end_game();
    messages_for_new_connection_.clear();
    send_message_to_all(sth);
}

