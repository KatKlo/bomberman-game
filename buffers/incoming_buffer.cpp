#include "incoming_buffer.h"

void IncomingBuffer::add_to_buffer(std::vector<uint8_t> &data, buffer_size_t size) {
    resize_if_needed(size + size_);
    std::copy(&data[0], &data[size], &buffer_[size_]);
    size_ += size;
}

void IncomingBuffer::check_size(Buffer::buffer_size_t needed_size) {
    if (needed_size > size_) {
        throw std::length_error("message to short");
    }
}

uint8_t IncomingBuffer::read_uint8_t() {
    check_size(read_index + sizeof(uint8_t));
    uint8_t result = *(uint8_t *) (&buffer_[read_index]);
    read_index += sizeof(uint8_t);
    return result;
}

uint16_t IncomingBuffer::read_uint16_t() {
    check_size(read_index + sizeof(uint16_t));
    uint16_t result = *(uint16_t *) (&buffer_[read_index]);
    read_index += sizeof(uint16_t);
    return be16toh(result);
}

uint32_t IncomingBuffer::read_uint32_t() {
    check_size(read_index + sizeof(uint32_t));
    uint32_t result = *(uint32_t *) (&buffer_[read_index]);
    read_index += sizeof(uint32_t);
    return be32toh(result);
}

std::string IncomingBuffer::read_string() {
    uint8_t str_length = read_uint8_t();
    check_size(read_index + str_length);
    std::string result((char *) &buffer_[read_index], str_length);
    read_index += str_length;
    return result;
}

Player IncomingBuffer::read_player() {
    auto player_name = read_string();
    auto player_address = read_string();

    return Player{player_name, player_address};
}

Position IncomingBuffer::read_position() {
    auto x = read_uint16_t();
    auto y = read_uint16_t();

    return Position{x, y};
}

Event::BombPlacedEvent IncomingBuffer::read_bomb_placed_event() {
    auto id = read_uint32_t();
    auto position = read_position();

    return Event::BombPlacedEvent{id, position};
}

Event::BombExplodedEvent IncomingBuffer::read_bomb_exploded_event() {
    auto id = read_uint32_t();
    auto robots_destroyed = read_players_id_vector();
    auto blocks_destroyed = read_positions_vector();

    return Event::BombExplodedEvent{id, robots_destroyed, blocks_destroyed};
}

Event::PlayerMovedEvent IncomingBuffer::read_player_moved_event() {
    auto id = read_uint8_t();
    auto position = read_position();

    return Event::PlayerMovedEvent{id, position};
}

Event::BlockPlacedEvent IncomingBuffer::read_block_placed_event() {
    auto position = read_position();

    return Event::BlockPlacedEvent{position};
}

Event::event_message_variant IncomingBuffer::read_event() {
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

std::vector<Event::event_message_variant> IncomingBuffer::read_events_vector() {
    auto vec_size = read_uint32_t();
    std::vector<Event::event_message_variant> vector;

    for (size_t i = 0; i < vec_size; i++) {
        auto event = read_event();
        vector.emplace_back(event);
    }

    return vector;
}

std::vector<player_id_t> IncomingBuffer::read_players_id_vector() {
    auto vec_size = read_uint32_t();
    std::vector<player_id_t> vector;

    for (size_t i = 0; i < vec_size; i++) {
        auto player_id = read_uint8_t();
        vector.emplace_back(player_id);
    }

    return vector;
}

std::vector<Position> IncomingBuffer::read_positions_vector() {
    auto vec_size = read_uint32_t();
    std::vector<Position> vector;

    for (size_t i = 0; i < vec_size; i++) {
        auto position = read_position();
        vector.emplace_back(position);
    }

    return vector;
}

std::unordered_map<player_id_t, Player> IncomingBuffer::read_players_map() {
    auto map_size = read_uint32_t();
    std::unordered_map<player_id_t, Player> map;

    for (size_t i = 0; i < map_size; i++) {
        auto player_id = read_uint8_t();
        auto player = read_player();
        map.emplace(player_id, player);
    }

    return map;
}

std::unordered_map<player_id_t, score_t> IncomingBuffer::read_player_scores_map() {
    auto map_size = read_uint32_t();
    std::unordered_map<player_id_t, score_t> map;

    for (size_t i = 0; i < map_size; i++) {
        auto player_id = read_uint8_t();
        auto score = read_uint32_t();
        map.emplace(player_id, score);
    }

    return map;
}

IncomingBuffer::IncomingBuffer() : Buffer(0), read_index(0) {}

