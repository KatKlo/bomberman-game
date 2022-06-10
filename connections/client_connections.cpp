#include "client_connections.h"
#include "../logger.h"

using udp = boost::asio::ip::udp;
using tcp = boost::asio::ip::tcp;
using namespace std;

Client::Client(boost::asio::io_context &io_context, ClientParameters &parameters) : gui_connection_(),
                                                                                    server_connection_(),
                                                                                    gameInfo_(parameters.get_player_name()) {

    gui_connection_ = make_shared<GuiConnection>(io_context, parameters.get_gui_address(),
                                                 parameters.get_port(), *this);

    server_connection_ = make_shared<ServerConnection>(io_context, parameters.get_server_address(), *this);

}

void Client::handle_input_message(InputMessage::input_message &&msg) {
    ClientMessage::client_message_optional new_msg = gameInfo_.handle_GUI_message(msg);

    if (new_msg.has_value()) {
        server_connection_->send(new_msg.value());
    }
}

void Client::handle_server_message(ServerMessage::server_message &&msg) {
    DrawMessage::draw_message_optional new_msg = gameInfo_.handle_server_message(msg);

    if (new_msg.has_value()) {
        gui_connection_->send(new_msg.value());
    }
}

Client::~Client() {
    Logger::print_debug("closing client connections");

    gui_connection_->close();
    server_connection_->close();
}

GuiConnection::GuiConnection(boost::asio::io_context &io_context, Address &&gui_address,
                             uint16_t port, Client &client) : Connection(),
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

void GuiConnection::send(DrawMessage::draw_message &msg) {
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
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    Logger::print_debug("read message from gui - ", length, " bytes");
                    read_msg_.add_packet(buffer_, length);

                    try {
                        client_.handle_input_message(read_msg_.read_input_message());
                    } catch (exception &e) { // it's udp - ignore incorrect messages
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
            [this](boost::system::error_code ec, size_t) {
                OutgoingBuffer sent_msg = write_msgs_.front();
                write_msgs_.pop_front();

                if (!ec) {
                    Logger::print_debug("send message to gui - ", sent_msg.size(), " bytes: ", sent_msg);

                    if (!write_msgs_.empty()) {
                        do_write_message();
                    }
                } else {
                    Logger::print_debug("error in writing to gui");
                }
            });
}

ServerConnection::ServerConnection(boost::asio::io_context &io_context, Address &&server_address,
                                   Client &client) : TCPConnection(boost::asio::ip::tcp::socket(io_context)),
                                                     client_(client) {
    Logger::print_debug("creating server connection");

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(server_address.host, server_address.port);

    boost::asio::connect(socket_, endpoints);
    socket_.set_option(tcp::no_delay(true));

    set_proper_address();

    Logger::print_debug("server connected - sending on address ", address_,
                        " receiving on address ", socket_.local_endpoint());

    do_read_message();
}

void ServerConnection::handle_messages_in_bufor() {
    bool is_sth_to_read_in_buffer = true;

    while (is_sth_to_read_in_buffer) {
        try {
            client_.handle_server_message(read_msg_.read_server_message());
        } catch (length_error &e) { // invalid argument should break whole program
            is_sth_to_read_in_buffer = false;
        }
    }
}

void ServerConnection::send(ClientMessage::client_message &msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.emplace_back(msg);

    if (!write_in_progress) {
        do_write_message();
    }
}

void ServerConnection::handle_connection_error() {
    throw domain_error("error in tcp connection with server on " + address_);
}