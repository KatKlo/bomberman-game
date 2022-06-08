#include "buffer.h"
#include <iostream>

// Buffer

std::ostream &operator<<(std::ostream &os, const Buffer &buffer) {
    for (int i = 0; i < buffer.size_; i++) {
        os << (uint32_t) buffer.buffer_[i] << " ";
    }
    return os;
}

void Buffer::resize_if_needed(Buffer::buffer_size_t needed_size) {
    if (needed_size > capacity_) {
        capacity_ = needed_size;
        buffer_.resize(capacity_, 0);
    }
}

Buffer::Buffer(unsigned long capacity) : buffer_(capacity), capacity_(capacity), size_(0) {}

// InputBuffer

void InputBuffer::add_to_buffer(std::vector<uint8_t> &data, buffer_size_t size) {
    resize_if_needed(size + size_);
    std::copy(&data[0], &data[size], &buffer_[size_]);
    size_ += size;
}

void InputBuffer::check_size(Buffer::buffer_size_t needed_size) {
    if (needed_size > size_) {
        throw std::length_error("message to short");
    }
}

uint8_t InputBuffer::read_uint8_t() {
    check_size(read_index + sizeof(uint8_t));
    uint8_t result = *(uint8_t *) (&buffer_[read_index]);
    read_index += sizeof(uint8_t);
    return result;
}

uint16_t InputBuffer::read_uint16_t() {
    check_size(read_index + sizeof(uint16_t));
    uint16_t result = *(uint16_t *) (&buffer_[read_index]);
    read_index += sizeof(uint16_t);
    return be16toh(result);
}

uint32_t InputBuffer::read_uint32_t() {
    check_size(read_index + sizeof(uint32_t));
    uint32_t result = *(uint32_t *) (&buffer_[read_index]);
    read_index += sizeof(uint32_t);
    return be32toh(result);
}

std::string InputBuffer::read_string() {
    uint8_t str_length = read_uint8_t();
    check_size(read_index + str_length);
    std::string result((char *) &buffer_[read_index], str_length);
    read_index += str_length;
    return result;
}

Player InputBuffer::read_player() {
    auto player_name = read_string();
    auto player_address = read_string();

    return Player{player_name, player_address};
}

Position InputBuffer::read_position() {
    auto x = read_uint16_t();
    auto y = read_uint16_t();

    return Position{x, y};
}

Event::BombPlacedEvent InputBuffer::read_bomb_placed_event() {
    auto id = read_uint32_t();
    auto position = read_position();

    return Event::BombPlacedEvent{id, position};
}

Event::BombExplodedEvent InputBuffer::read_bomb_exploded_event() {
    auto id = read_uint32_t();
    auto robots_destroyed = read_players_id_vector();
    auto blocks_destroyed = read_positions_vector();

    return Event::BombExplodedEvent{id, robots_destroyed, blocks_destroyed};
}

Event::PlayerMovedEvent InputBuffer::read_player_moved_event() {
    auto id = read_uint8_t();
    auto position = read_position();

    return Event::PlayerMovedEvent{id, position};
}

Event::BlockPlacedEvent InputBuffer::read_block_placed_event() {
    auto position = read_position();

    return Event::BlockPlacedEvent{position};
}

Event::event_message_variant InputBuffer::read_event() {
    switch (read_uint8_t()) {
        case Event::BOMB_PLACED: {
            return read_bomb_placed_event();
        }
        case Event::BOMB_EXPLODED: {
            return read_bomb_exploded_event();
        }
        case Event::PLAYER_MOVED: {
            return read_player_moved_event();
        }
        case Event::BLOCK_PLACED: {
            return read_block_placed_event();
        }
        default: {
            throw std::invalid_argument("bad event number");
        }
    }
}

std::vector<Event::event_message_variant> InputBuffer::read_events_vector() {
    auto vec_size = read_uint32_t();
    std::vector<Event::event_message_variant> vector;

    for (int i = 0; i < vec_size; i++) {
        auto event = read_event();
        vector.emplace_back(event);
    }

    return vector;
}

std::vector<player_id_t> InputBuffer::read_players_id_vector() {
    auto vec_size = read_uint32_t();
    std::vector<player_id_t> vector;

    for (int i = 0; i < vec_size; i++) {
        auto player_id = read_uint8_t();
        vector.emplace_back(player_id);
    }

    return vector;
}

std::vector<Position> InputBuffer::read_positions_vector() {
    auto vec_size = read_uint32_t();
    std::vector<Position> vector;

    for (int i = 0; i < vec_size; i++) {
        auto position = read_position();
        vector.emplace_back(position);
    }

    return vector;
}

std::unordered_map<player_id_t, Player> InputBuffer::read_players_map() {
    auto map_size = read_uint32_t();
    std::unordered_map<player_id_t, Player> map;

    for (int i = 0; i < map_size; i++) {
        auto player_id = read_uint8_t();
        auto player = read_player();
        map.emplace(player_id, player);
    }

    return map;
}

std::unordered_map<player_id_t, score_t> InputBuffer::read_player_scores_map() {
    auto map_size = read_uint32_t();
    std::unordered_map<player_id_t, score_t> map;

    for (int i = 0; i < map_size; i++) {
        auto player_id = read_uint8_t();
        auto score = read_uint32_t();
        map.emplace(player_id, score);
    }

    return map;
}

InputBuffer::InputBuffer() : Buffer(0), read_index(0) {}

// OutputBuffer

Buffer::buffer_size_t OutputBuffer::size() {
    return size_;
}

uint8_t *OutputBuffer::get_buffer() {
    return (uint8_t *) &buffer_[0];
}

void OutputBuffer::write_uint8_t(uint8_t number) {
    resize_if_needed(write_index + sizeof(uint8_t));
    *(uint8_t *) (&buffer_[write_index]) = number;
    write_index += sizeof(uint8_t);
}

void OutputBuffer::write_uint16_t(uint16_t number) {
    resize_if_needed(write_index + sizeof(uint16_t));
    *(uint16_t *) (&buffer_[write_index]) = htobe16(number);
    write_index += sizeof(uint16_t);
}

void OutputBuffer::write_uint32_t(uint32_t number) {
    resize_if_needed(write_index + sizeof(uint32_t));
    *(uint32_t *) (&buffer_[write_index]) = htobe32(number);
    write_index += sizeof(uint32_t);
}

void OutputBuffer::write_string(std::string &string) {
    write_uint8_t(string.length());
    resize_if_needed(write_index + string.length());
    std::copy(string.c_str(), string.c_str() + string.length(), &buffer_[write_index]);
    write_index += string.length();
}

void OutputBuffer::write_player(Player &player) {
    write_string(player.name);
    write_string(player.address);
}

void OutputBuffer::write_position(Position &position) {
    write_uint16_t(position.x);
    write_uint16_t(position.y);
}

void OutputBuffer::write_bomb(Bomb &bomb) {
    write_position(bomb.position);
    write_uint16_t(bomb.timer);
}

void OutputBuffer::write_bomb_placed_event(Event::BombPlacedEvent &event) {
    write_uint8_t(Event::BOMB_PLACED);
    write_uint32_t(event.id);
    write_position(event.position);
}

void OutputBuffer::write_bomb_exploded_event(Event::BombExplodedEvent &event) {
    write_uint8_t(Event::BOMB_EXPLODED);
    write_uint32_t(event.id);
    write_players_id_vector(event.robots_destroyed);
    write_positions_vector(event.blocks_destroyed);
}

void OutputBuffer::write_player_moved_event(Event::PlayerMovedEvent &event) {
    write_uint8_t(Event::PLAYER_MOVED);
    write_uint8_t(event.id);
    write_position(event.position);
}

void OutputBuffer::write_block_placed_event(Event::BlockPlacedEvent &event) {
    write_uint8_t(Event::BLOCK_PLACED);
    write_position(event.position);
}

void OutputBuffer::write_event(Event::event_message_variant &event) {
    switch (event.index()) {
        case Event::BOMB_PLACED: {
            write_bomb_placed_event(std::get<Event::BombPlacedEvent>(event));
            break;
        }
        case Event::BOMB_EXPLODED: {
            write_bomb_exploded_event(std::get<Event::BombExplodedEvent>(event));
            break;
        }
        case Event::PLAYER_MOVED: {
            write_player_moved_event(std::get<Event::PlayerMovedEvent>(event));
            break;
        }
        case Event::BLOCK_PLACED: {
            write_block_placed_event(std::get<Event::BlockPlacedEvent>(event));
            break;
        }
        default: {
            throw std::invalid_argument("bad event number");
        }
    }
}

void OutputBuffer::write_players_id_vector(std::vector<player_id_t> &player_ids) {
    write_uint32_t(player_ids.size());
    for (auto &player_id : player_ids) {
        write_uint8_t(player_id);
    }
}

void OutputBuffer::write_positions_vector(std::vector<Position> &positions) {
    write_uint32_t(positions.size());
    for (auto &it: positions) {
        write_position(it);
    }
}

void OutputBuffer::write_bombs_vector(std::vector<Bomb> &bombs) {
    write_uint32_t(bombs.size());
    for (auto &it: bombs) {
        write_bomb(it);
    }
}

void OutputBuffer::write_events_vector(std::vector<Event::event_message_variant> &events) {
    write_uint32_t(events.size());
    for (auto &it: events) {
        write_event(it);
    }
}

void OutputBuffer::write_players_map(std::unordered_map<player_id_t, Player> &players) {
    write_uint32_t(players.size());
    for (auto &it: players) {
        write_uint8_t(it.first);
        write_player(it.second);
    }
}

void OutputBuffer::write_player_positions_map(std::unordered_map<player_id_t, Position> &player_positions) {
    write_uint32_t(player_positions.size());
    for (auto &it: player_positions) {
        write_uint8_t(it.first);
        write_position(it.second);
    }
}

void OutputBuffer::write_player_scores_map(std::unordered_map<player_id_t, score_t> &scores) {
    write_uint32_t(scores.size());
    for (auto &it: scores) {
        write_uint8_t(it.first);
        write_uint32_t(it.second);
    }
}

void OutputBuffer::write_client_join_message(ClientMessage::Join &msg) {
    write_uint8_t(ClientMessage::JOIN);
    write_string(msg.name);
}

void OutputBuffer::write_client_place_bomb_message(ClientMessage::PlaceBomb &msg) {
    write_uint8_t(ClientMessage::PLACE_BOMB);
}

void OutputBuffer::write_client_place_block_message(ClientMessage::PlaceBlock &msg) {
    write_uint8_t(ClientMessage::PLACE_BLOCK);
}

void OutputBuffer::write_client_move_message(ClientMessage::Move &msg) {
    write_uint8_t(ClientMessage::MOVE);
    write_uint8_t(static_cast<uint8_t>(msg.direction));
}

void OutputBuffer::write_draw_lobby_message(DrawMessage::Lobby &msg) {
    write_uint8_t(DrawMessage::LOBBY);
    write_string(msg.server_name);
    write_uint8_t(msg.players_count);
    write_uint16_t(msg.size_x);
    write_uint16_t(msg.size_y);
    write_uint16_t(msg.game_length);
    write_uint16_t(msg.explosion_radius);
    write_uint16_t(msg.bomb_timer);
    write_players_map(msg.players);
}

void OutputBuffer::write_draw_game_message(DrawMessage::Game &msg) {
    write_uint8_t(DrawMessage::GAME);
    write_string(msg.server_name);
    write_uint16_t(msg.size_x);
    write_uint16_t(msg.size_y);
    write_uint16_t(msg.game_length);
    write_uint16_t(msg.turn);
    write_players_map(msg.players);
    write_player_positions_map(msg.player_positions);
    write_positions_vector(msg.blocks);
    write_bombs_vector(msg.bombs);
    write_positions_vector(msg.explosions);
    write_player_scores_map(msg.scores);
}

void OutputBuffer::write_server_hello_message(ServerMessage::Hello &msg) {
    write_uint8_t(ServerMessage::HELLO);
    write_string(msg.server_name);
    write_uint16_t(msg.size_x);
    write_uint16_t(msg.size_y);
    write_uint16_t(msg.game_length);
    write_uint16_t(msg.explosion_radius);
    write_uint16_t(msg.bomb_timer);
}

void OutputBuffer::write_server_accepted_player_message(ServerMessage::AcceptedPlayer &msg) {
    write_uint8_t(ServerMessage::ACCEPTED_PLAYER);
    write_uint8_t(msg.id);
    write_player(msg.player);
}

void OutputBuffer::write_server_game_started_message(ServerMessage::GameStarted &msg) {
    write_uint8_t(ServerMessage::GAME_STARTED);
    write_players_map(msg.players);
}

void OutputBuffer::write_server_turn_message(ServerMessage::Turn &msg) {
    write_uint8_t(ServerMessage::TURN);
    write_uint16_t(msg.turn);
    write_events_vector(msg.events);
}


void OutputBuffer::write_server_game_ended_message(ServerMessage::GameEnded &msg) {
    write_uint8_t(ServerMessage::GAME_ENDED);
    write_player_scores_map(msg.scores);
}


OutputBuffer::OutputBuffer(ClientMessage::client_message_variant &msg) : Buffer(MAX_PACKET_LENGTH), write_index(0) {
    switch (msg.index()) {
        case ClientMessage::JOIN:
            write_client_join_message(std::get<ClientMessage::Join>(msg));
            break;
        case ClientMessage::PLACE_BOMB:
            write_client_place_bomb_message(std::get<ClientMessage::PlaceBomb>(msg));
            break;
        case ClientMessage::PLACE_BLOCK:
            write_client_place_block_message(std::get<ClientMessage::PlaceBlock>(msg));
            break;
        case ClientMessage::MOVE:
            write_client_move_message(std::get<ClientMessage::Move>(msg));
            break;
    }

    size_ = write_index;
}

OutputBuffer::OutputBuffer(DrawMessage::draw_message_variant &msg) : Buffer(MAX_PACKET_LENGTH), write_index(0) {
    switch (msg.index()) {
        case DrawMessage::LOBBY:
            write_draw_lobby_message(std::get<DrawMessage::Lobby>(msg));
            break;
        case DrawMessage::GAME:
            write_draw_game_message(std::get<DrawMessage::Game>(msg));
            break;
    }

    size_ = write_index;
}

OutputBuffer::OutputBuffer(ServerMessage::server_message_variant &msg) : Buffer(MAX_PACKET_LENGTH), write_index(0) {
    switch (msg.index()) {
        case ServerMessage::HELLO:
            write_server_hello_message(std::get<ServerMessage::Hello>(msg));
            break;
        case ServerMessage::ACCEPTED_PLAYER:
            write_server_accepted_player_message(std::get<ServerMessage::AcceptedPlayer>(msg));
            break;
        case ServerMessage::GAME_STARTED:
            write_server_game_started_message(std::get<ServerMessage::GameStarted>(msg));
            break;
        case ServerMessage::TURN:
            write_server_turn_message(std::get<ServerMessage::Turn>(msg));
            break;
        case ServerMessage::GAME_ENDED:
            write_server_game_ended_message(std::get<ServerMessage::GameEnded>(msg));
            break;
    }

    size_ = write_index;
}

// UdpInputBuffer

void UdpInputBuffer::add_packet(std::vector<uint8_t> &data, Buffer::buffer_size_t size) {
    reset_buffer();
    add_to_buffer(data, size);
}


void UdpInputBuffer::reset_buffer() {
    read_index = 0;
    size_ = 0;
}

InputMessage::input_message_variant UdpInputBuffer::read_input_message() {
    InputMessage::input_message_variant result;
    switch (read_uint8_t()) {
        case InputMessage::PLACE_BOMB: {
            result = read_input_place_bomb_message();
            break;
        }
        case InputMessage::PLACE_BLOCK: {
            result = read_input_place_block_message();
            break;
        }
        case InputMessage::MOVE: {
            result = read_input_move_message();
            break;
        }
        default: {
            throw std::invalid_argument("bad gui message type");
        }
    }

    if (read_index != size_) {
        throw std::invalid_argument("gui message too long");
    }

    reset_buffer();
    return result;
}

InputMessage::Move UdpInputBuffer::read_input_move_message() {
    auto direction_number = read_uint8_t();
    return InputMessage::Move{Direction{direction_number}};
}

InputMessage::PlaceBomb UdpInputBuffer::read_input_place_bomb_message() {
    return InputMessage::PlaceBomb{};
}

InputMessage::PlaceBlock UdpInputBuffer::read_input_place_block_message() {
    return InputMessage::PlaceBlock{};
}

UdpInputBuffer::UdpInputBuffer() : InputBuffer() {}

// TcpInputBuffer

void TcpInputBuffer::add_packet(std::vector<uint8_t> &data, Buffer::buffer_size_t size) {
    add_to_buffer(data, size);
}

void TcpInputBuffer::clean_after_correct_read() {
    std::copy(&buffer_[read_index], &buffer_[size_], &buffer_[0]);
    size_ -= read_index;
    read_index = 0;
}

ServerMessage::server_message_variant TcpInputBuffer::read_server_message() {
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

ClientMessage::client_message_variant TcpInputBuffer::read_client_message() {
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

ServerMessage::Hello TcpInputBuffer::read_server_hello_message() {
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

ServerMessage::AcceptedPlayer TcpInputBuffer::read_server_accepted_player_message() {
    player_id_t id = read_uint8_t();
    Player player = read_player();

    return ServerMessage::AcceptedPlayer{id, player};
}

ServerMessage::GameStarted TcpInputBuffer::read_server_game_started_message() {
    std::unordered_map<player_id_t, Player> players = read_players_map();

    return ServerMessage::GameStarted{players};
}

ServerMessage::Turn TcpInputBuffer::read_server_turn_message() {
    uint16_t turn = read_uint16_t();
    std::vector<Event::event_message_variant> events = read_events_vector();
    return ServerMessage::Turn{turn, events};
}

ServerMessage::GameEnded TcpInputBuffer::read_server_game_ended_message() {
    std::unordered_map<player_id_t, score_t> scores = read_player_scores_map();
    return ServerMessage::GameEnded{scores};
}

ClientMessage::Join TcpInputBuffer::read_client_join_message(){
    std::string name = read_string();
    return ClientMessage::Join{name};
}

ClientMessage::PlaceBomb TcpInputBuffer::read_client_place_bomb_message(){
    return ClientMessage::PlaceBomb{};
}

ClientMessage::PlaceBlock TcpInputBuffer::read_client_place_block_message(){
    return ClientMessage::PlaceBlock{};
}

ClientMessage::Move TcpInputBuffer::read_client_move_message(){
    uint8_t direction_number = read_uint8_t();
    return ClientMessage::Move{Direction{direction_number}};
}

TcpInputBuffer::TcpInputBuffer() : InputBuffer() {}
