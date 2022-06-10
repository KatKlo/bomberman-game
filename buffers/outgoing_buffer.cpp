#include "outgoing_buffer.h"

Buffer::buffer_size_t OutgoingBuffer::size() {
    return size_;
}

uint8_t *OutgoingBuffer::get_buffer() {
    return (uint8_t *) &buffer_[0];
}

void OutgoingBuffer::write_uint8_t(uint8_t number) {
    resize_if_needed(write_index + sizeof(uint8_t));
    *(uint8_t *) (&buffer_[write_index]) = number;
    write_index += sizeof(uint8_t);
}

void OutgoingBuffer::write_uint16_t(uint16_t number) {
    resize_if_needed(write_index + sizeof(uint16_t));
    *(uint16_t *) (&buffer_[write_index]) = htobe16(number);
    write_index += sizeof(uint16_t);
}

void OutgoingBuffer::write_uint32_t(uint32_t number) {
    resize_if_needed(write_index + sizeof(uint32_t));
    *(uint32_t *) (&buffer_[write_index]) = htobe32(number);
    write_index += sizeof(uint32_t);
}

void OutgoingBuffer::write_string(std::string &string) {
    write_uint8_t(static_cast<uint8_t>(string.length()));
    resize_if_needed(write_index + string.length());
    std::copy(string.c_str(), string.c_str() + string.length(), &buffer_[write_index]);
    write_index += string.length();
}

void OutgoingBuffer::write_player(Player &player) {
    write_string(player.name);
    write_string(player.address);
}

void OutgoingBuffer::write_position(Position &position) {
    write_uint16_t(position.x);
    write_uint16_t(position.y);
}

void OutgoingBuffer::write_bomb(Bomb &bomb) {
    write_position(bomb.position);
    write_uint16_t(bomb.timer);
}

void OutgoingBuffer::write_bomb_placed_event(Event::BombPlacedEvent &event) {
    write_uint8_t(Event::BOMB_PLACED);
    write_uint32_t(event.id);
    write_position(event.position);
}

void OutgoingBuffer::write_bomb_exploded_event(Event::BombExplodedEvent &event) {
    write_uint8_t(Event::BOMB_EXPLODED);
    write_uint32_t(event.id);
    write_players_id_vector(event.robots_destroyed);
    write_positions_vector(event.blocks_destroyed);
}

void OutgoingBuffer::write_player_moved_event(Event::PlayerMovedEvent &event) {
    write_uint8_t(Event::PLAYER_MOVED);
    write_uint8_t(event.id);
    write_position(event.position);
}

void OutgoingBuffer::write_block_placed_event(Event::BlockPlacedEvent &event) {
    write_uint8_t(Event::BLOCK_PLACED);
    write_position(event.position);
}

void OutgoingBuffer::write_event(Event::event_message_variant &event) {
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

void OutgoingBuffer::write_players_id_vector(std::vector<player_id_t> &player_ids) {
    write_uint32_t(static_cast<uint32_t>(player_ids.size()));
    for (auto &player_id : player_ids) {
        write_uint8_t(player_id);
    }
}

void OutgoingBuffer::write_positions_vector(std::vector<Position> &positions) {
    write_uint32_t(static_cast<uint32_t>(positions.size()));
    for (auto &it: positions) {
        write_position(it);
    }
}

void OutgoingBuffer::write_bombs_vector(std::vector<Bomb> &bombs) {
    write_uint32_t(static_cast<uint32_t>(bombs.size()));
    for (auto &it: bombs) {
        write_bomb(it);
    }
}

void OutgoingBuffer::write_events_vector(std::vector<Event::event_message_variant> &events) {
    write_uint32_t(static_cast<uint32_t>(events.size()));
    for (auto &it: events) {
        write_event(it);
    }
}

void OutgoingBuffer::write_players_map(std::unordered_map<player_id_t, Player> &players) {
    write_uint32_t(static_cast<uint32_t>(players.size()));
    for (auto &it: players) {
        write_uint8_t(it.first);
        write_player(it.second);
    }
}

void OutgoingBuffer::write_player_positions_map(std::unordered_map<player_id_t, Position> &player_positions) {
    write_uint32_t(static_cast<uint32_t>(player_positions.size()));
    for (auto &it: player_positions) {
        write_uint8_t(it.first);
        write_position(it.second);
    }
}

void OutgoingBuffer::write_player_scores_map(std::unordered_map<player_id_t, score_t> &scores) {
    write_uint32_t(static_cast<uint32_t>(scores.size()));
    for (auto &it: scores) {
        write_uint8_t(it.first);
        write_uint32_t(it.second);
    }
}

void OutgoingBuffer::write_client_join_message(ClientMessage::Join &msg) {
    write_uint8_t(ClientMessage::JOIN);
    write_string(msg.name);
}

void OutgoingBuffer::write_client_place_bomb_message() {
    write_uint8_t(ClientMessage::PLACE_BOMB);
}

void OutgoingBuffer::write_client_place_block_message() {
    write_uint8_t(ClientMessage::PLACE_BLOCK);
}

void OutgoingBuffer::write_client_move_message(ClientMessage::Move &msg) {
    write_uint8_t(ClientMessage::MOVE);
    write_uint8_t(static_cast<uint8_t>(msg.direction));
}

void OutgoingBuffer::write_draw_lobby_message(DrawMessage::Lobby &msg) {
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

void OutgoingBuffer::write_draw_game_message(DrawMessage::Game &msg) {
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

void OutgoingBuffer::write_server_hello_message(ServerMessage::Hello &msg) {
    write_uint8_t(ServerMessage::HELLO);
    write_string(msg.server_name);
    write_uint8_t(msg.players_count);
    write_uint16_t(msg.size_x);
    write_uint16_t(msg.size_y);
    write_uint16_t(msg.game_length);
    write_uint16_t(msg.explosion_radius);
    write_uint16_t(msg.bomb_timer);
}

void OutgoingBuffer::write_server_accepted_player_message(ServerMessage::AcceptedPlayer &msg) {
    write_uint8_t(ServerMessage::ACCEPTED_PLAYER);
    write_uint8_t(msg.id);
    write_player(msg.player);
}

void OutgoingBuffer::write_server_game_started_message(ServerMessage::GameStarted &msg) {
    write_uint8_t(ServerMessage::GAME_STARTED);
    write_players_map(msg.players);
}

void OutgoingBuffer::write_server_turn_message(ServerMessage::Turn &msg) {
    write_uint8_t(ServerMessage::TURN);
    write_uint16_t(msg.turn);
    write_events_vector(msg.events);
}


void OutgoingBuffer::write_server_game_ended_message(ServerMessage::GameEnded &msg) {
    write_uint8_t(ServerMessage::GAME_ENDED);
    write_player_scores_map(msg.scores);
}


OutgoingBuffer::OutgoingBuffer(ClientMessage::client_message_variant &msg) : Buffer(MAX_PACKET_LENGTH), write_index(0) {
    switch (msg.index()) {
        case ClientMessage::JOIN:
            write_client_join_message(std::get<ClientMessage::Join>(msg));
            break;
        case ClientMessage::PLACE_BOMB:
            write_client_place_bomb_message();
            break;
        case ClientMessage::PLACE_BLOCK:
            write_client_place_block_message();
            break;
        case ClientMessage::MOVE:
            write_client_move_message(std::get<ClientMessage::Move>(msg));
            break;
    }

    size_ = write_index;
}

OutgoingBuffer::OutgoingBuffer(DrawMessage::draw_message_variant &msg) : Buffer(MAX_PACKET_LENGTH), write_index(0) {
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

OutgoingBuffer::OutgoingBuffer(ServerMessage::server_message_variant &msg) : Buffer(MAX_PACKET_LENGTH), write_index(0) {
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