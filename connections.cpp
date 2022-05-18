#include "connections.h"
#include <boost/asio.hpp>
#include "structures.h"
#include "buffer.h"
#include "utils.h"

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

GUIConnection::GUIConnection(boost::asio::io_context &io_context, Address &&gui_address, uint16_t port, Client &client)
        : client_(client), socket(io_context, udp::endpoint(udp::v4(), port)) {
    Logger::print_debug("connecting to GUI server (", gui_address, ") on port ", port);

    boost::asio::ip::udp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(gui_address.host, gui_address.port);
    boost::asio::connect(socket, endpoints);

    Logger::print_debug("GUI server connected");

    Logger::print_debug("starting GUI reading");
    do_read_message();
}

void GUIConnection::send(ClientMessages::Client_GUI_message_variant &msg) {
    Logger::print_debug("starting sending to GUI");
    bool write_in_progress = !write_msgs.empty();
    OutputBuffer o;
    o.write_client_to_GUI_message(msg);
    write_msgs.push_back(o);
    if (!write_in_progress) {
        do_write();
    }
}

void GUIConnection::do_read_message() {
    socket.async_receive(
            boost::asio::buffer(read_msg.get_buffer(), Buffer::MAX_UDP_LENGTH),
            [this](boost::system::error_code ec, std::size_t length) {
                Logger::print_debug("GUI - do read ", length);
                if (!ec) {
                    read_msg.set_size(length);
                    Logger::print_debug("Received message from gui:\n", read_msg);
                    try {
                        auto sth = read_msg.read_GUI_message();
                        client_.handle_GUI_message(sth);
                    } catch (std::invalid_argument &e) {
                        Logger::print_debug("Bad message from gui - ", e.what());
                    }
                    do_read_message();
                } else {
                    // TODO realizacja erroru
                }
            });
}

void GUIConnection::do_write() {
    socket.async_send(
            boost::asio::buffer(write_msgs.front().get_buffer(), write_msgs.front().size()),
            [this](boost::system::error_code ec, std::size_t /*length*/) {
                Logger::print_debug("GUI - do write:\n", write_msgs.front());
                if (!ec) {
                    write_msgs.pop_front();
                    if (!write_msgs.empty()) {
                        do_write();
                    }
                } else {
                    // TODO realizacja erroru
                }
            });

}

ServerConnection::ServerConnection(boost::asio::io_context &io_context, Address &&server_address, Client &client)
        : client_(client), socket(io_context) {
    Logger::print_debug("connecting to server (", server_address, ")");


    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(server_address.host, server_address.port);
//    do_connect(endpoints);
    boost::asio::connect(socket, endpoints);
    Logger::print_debug("server connected");
    boost::asio::ip::tcp::no_delay option(true);
    socket.set_option(option);
    Logger::print_debug("starting server reading");
    do_read_message();
}

void ServerConnection::do_read_message() {
    socket.async_read_some(
            boost::asio::buffer(read_msg.get_buffer(), Buffer::MAX_UDP_LENGTH),
            [this](boost::system::error_code ec, std::size_t length) {
                Logger::print_debug("server - do read ", length);
                if (!ec) {
                    read_msg.set_size(length);
                    Logger::print_debug("Received message from server:\n", read_msg);
                    try {
                        auto sth = read_msg.read_server_message();
                        client_.handle_server_message(sth);
                    } catch (std::invalid_argument &e) {
                        Logger::print_debug("Bad message from server - ", e.what());
                    }
                    do_read_message();
                } else {
                    // TODO realizacja erroru
                }
            });
}

void ServerConnection::do_write() {
    socket.async_send(
            boost::asio::buffer(write_msgs.front().get_buffer(), write_msgs.front().size()),
            [this](boost::system::error_code ec, std::size_t /*length*/) {
                Logger::print_debug("server - do write:\n", write_msgs.front());
                if (!ec) {
                    write_msgs.pop_front();
                    if (!write_msgs.empty()) {
                        do_write();
                    }
                } else {
                    // TODO realizacja erroru
                }
            });
}

void ServerConnection::send(ClientMessages::Client_server_message_variant &msg) {
    Logger::print_debug("starting sending to server");
    bool write_in_progress = !write_msgs.empty();
    OutputBuffer o;
    o.write_client_to_server_message(msg);
    write_msgs.push_back(o);
    if (!write_in_progress) {
        do_write();
    }
}

void ServerConnection::do_connect(const tcp::resolver::results_type &endpoints) {
    boost::asio::async_connect(socket, endpoints,
                               [this](boost::system::error_code ec, const tcp::endpoint &) {
                                   if (!ec) {
                                       Logger::print_debug("server connected");
                                       Logger::print_debug("starting server reading");
                                       do_read_message();
                                   }
                               });
}

Client::Client(boost::asio::io_context &io_context, ClientParameters &parameters) {
    server_connection = std::make_shared<ServerConnection>(io_context,
                                                           parameters.get_server_address(),
                                                           *this);

    gui_connection = std::make_shared<GUIConnection>(io_context,
                                                     parameters.get_gui_address(),
                                                     parameters.get_port(),
                                                     *this);

    gameInfo = std::nullopt;
    name = parameters.get_player_name();
}

void Client::handle_GUI_message(GUIMessages::GUI_message_variant &msg) {
    if (!gameInfo.has_value()) {
        Logger::print_debug("no connection with server");
        return;
    }

    ClientMessages::Client_server_message_optional_variant new_msg = gameInfo.value().handle_GUI_message(msg);
    if (new_msg.has_value()) {
        server_connection->send(new_msg.value());
    }
}

void Client::handle_server_message(ServerMessage::Server_message_variant &msg) {
    if (!gameInfo.has_value() && msg.index() == ServerMessage::HELLO) {
        gameInfo = GameInfo(std::get<ServerMessage::HelloMessage>(msg), name);
    }

    if (gameInfo.has_value()) {
        auto new_msg = gameInfo.value().handle_server_message(msg);
        if (new_msg.has_value()) {
            gui_connection->send(new_msg.value());
        }
    }
}
