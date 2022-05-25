#include "connections.h"
#include <boost/asio.hpp>
#include "structures.h"
#include "buffer.h"
#include "utils.h"

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

GUIConnection::GUIConnection(boost::asio::io_context &io_context, Address &&gui_address, uint16_t port, Client &client)
        : client_(client),
          socket(io_context, udp::endpoint(udp::v6(), port)),
          remote_endpoint_(),
          read_msg(),
          write_msg(),
          buffer_(Buffer::MAX_PACKET_LENGTH) {
    Logger::print_debug("connecting to GUI server (", gui_address, ") on port ", port);

    boost::asio::ip::udp::resolver resolver(io_context);
    remote_endpoint_ = *resolver.resolve(gui_address.host, gui_address.port);
//    auto endpoints = resolver.resolve(gui_address.host, gui_address.port);
//    boost::asio::connect(socket, endpoints);

    Logger::print_debug("GUI server connected");

    Logger::print_debug("starting GUI reading");
    do_read_message();
}

void GUIConnection::send(DrawMessage::draw_message_variant &msg) {
    write_msg.write_draw_message(msg);
    Logger::print_debug("starting sending to GUI:\n", write_msg);
    socket.send_to(boost::asio::buffer(write_msg.get_buffer(), write_msg.size()), remote_endpoint_);
    Logger::print_debug("sent to GUI");
}

void GUIConnection::close() {
    socket.close();
}


void GUIConnection::do_read_message() {
    socket.async_receive_from(
            boost::asio::buffer(&buffer_[0], Buffer::MAX_PACKET_LENGTH),
            remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t length) {
                Logger::print_debug("GUI - do read ", length);
                if (!ec) {
                    read_msg.add_packet(buffer_, length);
                    Logger::print_debug("Received message from gui:\n", read_msg);
                    try {
                        auto sth = read_msg.read_input_message();
                        client_.handle_GUI_message(sth);
                    } catch (std::logic_error &e) {
                        Logger::print_debug("Bad message from gui - ", e.what());
                    }
                } else if (ec == boost::asio::error::operation_aborted) {
                    return;
                } else {
                    Logger::print_debug("Error in reading from gui");
                }
                do_read_message();
            }

    );
}

ServerConnection::ServerConnection(boost::asio::io_context &io_context, Address &&server_address, Client &client)
        : client_(client),
          socket(io_context),
          read_msg(),
          write_msg(),
          buffer_(Buffer::MAX_PACKET_LENGTH) {
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
            boost::asio::buffer(&buffer_[0], Buffer::MAX_PACKET_LENGTH),
            [this](boost::system::error_code ec, std::size_t length) {
                Logger::print_debug("server - do read ", length);
                if (!ec) {
                    read_msg.add_packet(buffer_, length);
                    Logger::print_debug("Received message from server:\n", read_msg);
                    bool aaa = true;
                    while (aaa) {
                        try {
                            auto sth = read_msg.read_server_message();
                            Logger::print_debug("read message :)");
                            client_.handle_server_message(sth);
                        } catch (std::length_error &e) {
                            aaa = false;
                        }
                    }
                    do_read_message();
                } else if (ec == boost::asio::error::operation_aborted) {
                    return;
                } else {
                    Logger::print_debug("Error in reading from server");
                    client_.close_connections();
                }
            });
}

void ServerConnection::send(ClientMessage::client_message_variant &msg) {
    write_msg.write_client_message(msg);
    Logger::print_debug("starting sending to server:\n", write_msg);
    socket.send(boost::asio::buffer(write_msg.get_buffer(), write_msg.size()));
    Logger::print_debug("sent to server");
}

void ServerConnection::close() {
    socket.close();
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

void Client::handle_GUI_message(InputMessage::input_message_variant &msg) {
    if (!gameInfo.has_value()) {
        Logger::print_debug("no connection with server");
        return;
    }

    ClientMessage::client_message_optional_variant new_msg = gameInfo.value().handle_GUI_message(msg);
    if (new_msg.has_value()) {
        server_connection->send(new_msg.value());
    }
}

void Client::handle_server_message(ServerMessage::server_message_variant &msg) {
    if (!gameInfo.has_value() && msg.index() == ServerMessage::HELLO) {
        gameInfo = std::make_optional<GameInfo>(std::get<ServerMessage::Hello>(msg), name);
    }

    if (gameInfo.has_value()) {
        auto new_msg = gameInfo.value().handle_server_message(msg);
        if (new_msg.has_value()) {
            gui_connection->send(new_msg.value());
        }
    }
}

void Client::close_connections() {
    gui_connection->close();
    server_connection->close();
}
