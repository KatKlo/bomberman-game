#include "tcp_incoming_buffer.h"

void TcpIncomingBuffer::add_packet(std::vector<uint8_t> &data, Buffer::buffer_size_t size) {
    add_to_buffer(data, size);
}

void TcpIncomingBuffer::clean_after_correct_read() {
    std::copy(&buffer_[read_index], &buffer_[size_], &buffer_[0]);
    size_ -= read_index;
    read_index = 0;
}

ServerMessage::server_message_variant TcpIncomingBuffer::read_server_message() {
    ServerMessage::server_message_variant result;
    read_index = 0;
    switch (read_uint8_t()) {
        case ServerMessage::HELLO: {
            result = read_server_hello_message();
            break;
        }
        case ServerMessage::ACCEPTED_PLAYER: {
            result = read_server_accepted_player_message();
            break;
        }
        case ServerMessage::GAME_STARTED: {
            result = read_server_game_started_message();
            break;
        }
        case ServerMessage::TURN: {
            result = read_server_turn_message();
            break;
        }
        case ServerMessage::GAME_ENDED: {
            result = read_server_game_ended_message();
            break;
        }
        default: {
            throw std::invalid_argument("bad server message type");
        }
    }

    clean_after_correct_read();
    return result;
}

ClientMessage::client_message_variant TcpIncomingBuffer::read_client_message() {
    ClientMessage::client_message_variant result;
    read_index = 0;
    switch (read_uint8_t()) {
        case ClientMessage::JOIN: {
            result = read_client_join_message();
            break;
        }
        case ClientMessage::PLACE_BOMB: {
            result = read_client_place_bomb_message();
            break;
        }
        case ClientMessage::PLACE_BLOCK: {
            result = read_client_place_block_message();
            break;
        }
        case ClientMessage::MOVE: {
            result = read_client_move_message();
            break;
        }
        default: {
            throw std::invalid_argument("bad client message type");
        }
    }

    clean_after_correct_read();
    return result;
}

ServerMessage::Hello TcpIncomingBuffer::read_server_hello_message() {
    std::string server_name = read_string();
    uint8_t players_count = read_uint8_t();
    uint16_t size_x = read_uint16_t();
    uint16_t size_y = read_uint16_t();
    uint16_t game_length = read_uint16_t();
    uint16_t explosion_radius = read_uint16_t();
    uint16_t bomb_timer = read_uint16_t();

    return ServerMessage::Hello{server_name, players_count, size_x, size_y,
                                game_length, explosion_radius, bomb_timer};
}

ServerMessage::AcceptedPlayer TcpIncomingBuffer::read_server_accepted_player_message() {
    player_id_t id = read_uint8_t();
    Player player = read_player();

    return ServerMessage::AcceptedPlayer{id, player};
}

ServerMessage::GameStarted TcpIncomingBuffer::read_server_game_started_message() {
    std::unordered_map<player_id_t, Player> players = read_players_map();

    return ServerMessage::GameStarted{players};
}

ServerMessage::Turn TcpIncomingBuffer::read_server_turn_message() {
    uint16_t turn = read_uint16_t();
    std::vector<Event::event_message_variant> events = read_events_vector();
    return ServerMessage::Turn{turn, events};
}

ServerMessage::GameEnded TcpIncomingBuffer::read_server_game_ended_message() {
    std::unordered_map<player_id_t, score_t> scores = read_player_scores_map();
    return ServerMessage::GameEnded{scores};
}

ClientMessage::Join TcpIncomingBuffer::read_client_join_message(){
    std::string name = read_string();
    return ClientMessage::Join{name};
}

ClientMessage::PlaceBomb TcpIncomingBuffer::read_client_place_bomb_message(){
    return ClientMessage::PlaceBomb{};
}

ClientMessage::PlaceBlock TcpIncomingBuffer::read_client_place_block_message(){
    return ClientMessage::PlaceBlock{};
}

ClientMessage::Move TcpIncomingBuffer::read_client_move_message(){
    uint8_t direction_number = read_uint8_t();
    return ClientMessage::Move{Direction{direction_number}};
}

TcpIncomingBuffer::TcpIncomingBuffer() : IncomingBuffer() {}

