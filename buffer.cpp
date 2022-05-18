#include <cstring>
#include "buffer.h"
#include "structures.h"

Buffer::Buffer() : buffer(), size_(0) {}

Buffer::buffer_size_t Buffer::size() {
    return size_;
}

void Buffer::set_size(Buffer::buffer_size_t new_size) {
    size_ = new_size;
}

char *Buffer::get_buffer() {
    return (char *) buffer;
}

std::ostream &operator<<(std::ostream &os, const Buffer &buffer) {
    uint8_t *buf = (uint8_t *) buffer.buffer;
    for (int i = 0; i < buffer.size_; i++) {
        os << (uint32_t) buf[i] << " ";
    }
    return os;
}

void InputBuffer::reset() {
    read_index = 0;
}

GUIMessages::GUI_message_variant InputBuffer::read_GUI_message() {
    reset();
    GUIMessages::GUI_message_variant result;
    switch (read_uint8_t()) {
        case GUIMessages::PLACE_BOMB: {
            result = read_gui_place_bomb_message();
            break;
        }
        case GUIMessages::PLACE_BLOCK: {
            result = read_gui_place_block_message();
            break;
        }
        case GUIMessages::MOVE: {
            result = read_gui_move_message();
            break;
        }
        default: {
            throw std::invalid_argument("bad gui message type");
        }
    }

    if (read_index != size_) {
        throw std::invalid_argument("gui message too big");
    }

    return result;
}

ServerMessage::Server_message_variant InputBuffer::read_server_message() {
//    Logger::print_debug("Read server message buff size: ", size_, " buff read_index: ", read_index);
    reset();
    ServerMessage::Server_message_variant result;
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

//    Logger::print_debug("buff size: ", size_, " buff read_index: ", read_index);
    if (read_index != size_) {
        throw std::invalid_argument("too big server message size");
    }

    return result;
}

uint8_t InputBuffer::read_uint8_t() {
    if (read_index + sizeof(uint8_t) > size_) {
        throw std::invalid_argument("bad read_uint8_t");
    }

    uint8_t result = *(uint8_t *) (buffer + read_index);
    read_index += sizeof(uint8_t);
    return result;
}

uint16_t InputBuffer::read_uint16_t() {
    if (read_index + sizeof(uint16_t) > size_) {
        throw std::invalid_argument("bad read_uint16_t");
    }

    uint16_t result = *(uint16_t *) (buffer + read_index);
    read_index += sizeof(uint16_t);
    return be16toh(result);
}

uint32_t InputBuffer::read_uint32_t() {
    if (read_index + sizeof(uint32_t) > size_) {
        throw std::invalid_argument("bad read_uint32_t");
    }

    uint32_t result = *(uint32_t *) (buffer + read_index);
    read_index += sizeof(uint32_t);
    return be32toh(result);
}

//uint64_t InputBuffer::read_uint64_t() {
//    if (read_index + sizeof(uint64_t) > size_) {
//        throw std::invalid_argument("bad read_uint64_t");
//    }
//
//    uint64_t result = *(uint64_t *) (buffer + read_index);
//    read_index += sizeof(uint64_t);
//    return be64toh(result);
//}

std::string InputBuffer::read_string() {
//    Logger::print_debug("Read string size so buff size: ", size_, " buff read_index: ", read_index);
    uint8_t str_length = read_uint8_t();
//    Logger::print_debug("Read string with size: ", str_length, " buff size: ", size_, " buff read_index: ", read_index);
    if (read_index + str_length > size_) {
        throw std::invalid_argument("bad read_string");
    }

    char c_string[str_length];
    memcpy(c_string, buffer + read_index, str_length);
    read_index += str_length;
    return std::string{c_string};
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

ServerMessage::BombPlacedEvent InputBuffer::read_bomb_placed_event() {
    auto id = read_uint32_t();
    auto position = read_position();

    return ServerMessage::BombPlacedEvent{id, position};
}

ServerMessage::BombExplodedEvent InputBuffer::read_bomb_exploded_event() {
    auto id = read_uint32_t();
    auto robots_destroyed = read_players_id_vector();
    auto blocks_destroyed = read_positions_vector();

    return ServerMessage::BombExplodedEvent{id, robots_destroyed, blocks_destroyed};
}

ServerMessage::PlayerMovedEvent InputBuffer::read_player_moved_event() {
    auto id = read_uint8_t();
    auto position = read_position();

    return ServerMessage::PlayerMovedEvent{id, position};
}

ServerMessage::BlockPlacedEvent InputBuffer::read_block_placed_event() {
    auto position = read_position();

    return ServerMessage::BlockPlacedEvent{position};
}

ServerMessage::event_message_variant InputBuffer::read_event() {
    switch (read_uint8_t()) {
        case ServerMessage::BOMB_PLACED: {
            return read_bomb_placed_event();
        }
        case ServerMessage::BOMB_EXPLODED: {
            return read_bomb_exploded_event();
        }
        case ServerMessage::PLAYER_MOVED: {
            return read_player_moved_event();
        }
        case ServerMessage::BLOCK_PLACED: {
            return read_block_placed_event();
        }
        default: {
            throw std::invalid_argument("bad event number");
        }
    }
}

std::vector<ServerMessage::event_message_variant> InputBuffer::read_events_vector() {
    auto vec_size = read_uint32_t();
    std::vector<ServerMessage::event_message_variant> vector(vec_size);

    for (int i = 0; i < vec_size; i++) {
        auto event = read_event();
        vector.emplace_back(event);
    }

    return vector;
}

std::vector<player_id_t> InputBuffer::read_players_id_vector() {
    auto vec_size = read_uint32_t();
    std::vector<player_id_t> vector(vec_size);

    for (int i = 0; i < vec_size; i++) {
        auto player_id = read_uint8_t();
        vector.emplace_back(player_id);
    }

    return vector;
}

std::vector<Position> InputBuffer::read_positions_vector() {
    auto vec_size = read_uint32_t();
    std::vector<Position> vector(vec_size);

    for (int i = 0; i < vec_size; i++) {
        auto position = read_position();
        vector.emplace_back(position);
    }

    return vector;
}

std::unordered_map<player_id_t, Player> InputBuffer::read_players_map() {
    auto map_size = read_uint32_t();
    std::unordered_map<player_id_t, Player> map(map_size);

    for (int i = 0; i < map_size; i++) {
        auto player_id = read_uint8_t();
        auto player = read_player();
        map.emplace(player_id, player);
    }

    return map;
}

std::unordered_map<player_id_t, score_t> InputBuffer::read_player_scores_map() {
    auto map_size = read_uint32_t();
    std::unordered_map<player_id_t, score_t> map(map_size);

    for (int i = 0; i < map_size; i++) {
        auto player_id = read_uint8_t();
        auto score = read_uint32_t();
        map.emplace(player_id, score);
    }

    return map;
}

GUIMessages::MoveMessage InputBuffer::read_gui_move_message() {
    auto direction_number = read_uint8_t();
    return GUIMessages::MoveMessage{Direction{direction_number}};
}

GUIMessages::PlaceBombMessage InputBuffer::read_gui_place_bomb_message() {
    return GUIMessages::PlaceBombMessage{};
}

GUIMessages::PlaceBlockMessage InputBuffer::read_gui_place_block_message() {
    return GUIMessages::PlaceBlockMessage{};
}

ServerMessage::HelloMessage InputBuffer::read_server_hello_message() {
//    Logger::print_debug("Read hello message buff size: ", size_, " buff read_index: ", read_index);
    std::string server_name = read_string();
    uint8_t players_count = read_uint8_t();
    uint16_t size_x = read_uint16_t();
    uint16_t size_y = read_uint16_t();
    uint16_t game_length = read_uint16_t();
    uint16_t explosion_radius = read_uint16_t();
    uint16_t bomb_timer = read_uint16_t();

    return ServerMessage::HelloMessage{server_name, players_count, size_x, size_y,
                                       game_length, explosion_radius, bomb_timer};
}

ServerMessage::AcceptedPlayerMessage InputBuffer::read_server_accepted_player_message() {
    player_id_t id = read_uint8_t();
    Player player = read_player();

    return ServerMessage::AcceptedPlayerMessage{id, player};
}

ServerMessage::GameStartedMessage InputBuffer::read_server_game_started_message() {
    std::unordered_map<player_id_t, Player> players = read_players_map();

    return ServerMessage::GameStartedMessage{players};
}

ServerMessage::TurnMessage InputBuffer::read_server_turn_message() {
    uint16_t turn = read_uint16_t();
    std::vector<ServerMessage::event_message_variant> events = read_events_vector();
    return ServerMessage::TurnMessage{turn, events};
}

ServerMessage::GameEndedMessage InputBuffer::read_server_game_ended_message() {
    std::unordered_map<player_id_t, score_t> scores = read_player_scores_map();
    return ServerMessage::GameEndedMessage{scores};
}

void OutputBuffer::reset() {
    write_index = 0;
    size_ = 0;
}

void OutputBuffer::write_client_to_GUI_message(ClientMessages::Client_GUI_message_variant &msg) {
    reset();
    switch (msg.index()) {
        case ClientMessages::LOBBY:
            write_client_lobby_message(std::get<ClientMessages::LobbyMessage>(msg));
            break;
        case ClientMessages::GAME:
            write_client_game_message(std::get<ClientMessages::GameMessage>(msg));
            break;
    }
    set_size(write_index);
}

void OutputBuffer::write_client_to_server_message(ClientMessages::Client_server_message_variant &msg) {
    reset();
    switch (msg.index()) {
        case ClientMessages::JOIN:
            write_client_join_message(std::get<ClientMessages::JoinMessage>(msg));
            break;
        case ClientMessages::PLACE_BOMB:
            write_client_place_bomb_message(std::get<ClientMessages::PlaceBombMessage>(msg));
            break;
        case ClientMessages::PLACE_BLOCK:
            write_client_place_block_message(std::get<ClientMessages::PlaceBlockMessage>(msg));
            break;
        case ClientMessages::MOVE:
            write_client_move_message(std::get<ClientMessages::MoveMessage>(msg));
            break;
    }
    set_size(write_index);
}

void OutputBuffer::write_uint8_t(uint8_t number) {
    *(uint8_t *) (buffer + write_index) = number;
    write_index += sizeof(uint8_t);
}

void OutputBuffer::write_uint16_t(uint16_t number) {
    *(uint16_t *) (buffer + write_index) = htobe16(number);
    write_index += sizeof(uint16_t);
}

void OutputBuffer::write_uint32_t(uint32_t number) {
    *(uint32_t *) (buffer + write_index) = htobe32(number);
    write_index += sizeof(uint32_t);
}

void OutputBuffer::write_uint64_t(uint64_t number) {
    *(uint64_t *) (buffer + write_index) = htobe64(number);
    write_index += sizeof(uint64_t);
}

void OutputBuffer::write_string(std::string &string) {
    write_uint8_t(string.length());
    memcpy(buffer + write_index, string.c_str(), string.length());
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

void OutputBuffer::write_client_join_message(ClientMessages::JoinMessage &msg) {
    write_uint8_t(ClientMessages::JOIN);
    write_string(msg.name);
}

void OutputBuffer::write_client_place_bomb_message(ClientMessages::PlaceBombMessage &msg) {
    write_uint8_t(ClientMessages::PLACE_BOMB);
}

void OutputBuffer::write_client_place_block_message(ClientMessages::PlaceBlockMessage &msg) {
    write_uint8_t(ClientMessages::PLACE_BLOCK);
}

void OutputBuffer::write_client_move_message(ClientMessages::MoveMessage &msg) {
    write_uint8_t(ClientMessages::MOVE);
    write_uint8_t(static_cast<uint8_t>(msg.direction));
}

void OutputBuffer::write_client_lobby_message(ClientMessages::LobbyMessage &msg) {
    write_uint8_t(ClientMessages::LOBBY);
    write_string(msg.server_name);
    write_uint8_t(msg.players_count);
    write_uint16_t(msg.size_x);
    write_uint16_t(msg.size_y);
    write_uint16_t(msg.game_length);
    write_uint16_t(msg.explosion_radius);
    write_uint16_t(msg.bomb_timer);
    write_players_map(msg.players);
}

void OutputBuffer::write_client_game_message(ClientMessages::GameMessage &msg) {
    write_uint8_t(ClientMessages::GAME);
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