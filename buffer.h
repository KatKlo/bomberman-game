#ifndef ROBOTS_BUFFER_H
#define ROBOTS_BUFFER_H

#include "structures.h"

// Superclass for all buffers-parsers
class Buffer {
public:
    using buffer_size_t = size_t;
    static constexpr auto MAX_PACKET_LENGTH = 65507;

    friend std::ostream &operator<<(std::ostream &os, const Buffer &buffer);

protected:
    std::vector<uint8_t> buffer_;
    buffer_size_t capacity_;
    buffer_size_t size_;

    explicit Buffer(buffer_size_t capacity);
    void resize_if_needed(buffer_size_t needed_size);
};

// Class for storing and parsing incoming messages
class InputBuffer : public Buffer {
public:
    virtual void add_packet(std::vector<uint8_t> &data, buffer_size_t size) = 0;

protected:
    buffer_size_t read_index;

    InputBuffer();
    void add_to_buffer(std::vector<uint8_t> &data, buffer_size_t size);

    uint8_t read_uint8_t();
    uint16_t read_uint16_t();
    uint32_t read_uint32_t();
    std::string read_string();

    Player read_player();
    Position read_position();

    Event::BombPlacedEvent read_bomb_placed_event();
    Event::BombExplodedEvent read_bomb_exploded_event();
    Event::PlayerMovedEvent read_player_moved_event();
    Event::BlockPlacedEvent read_block_placed_event();

    Event::event_message_variant read_event();

    std::vector<Event::event_message_variant> read_events_vector();
    std::vector<player_id_t> read_players_id_vector();
    std::vector<Position> read_positions_vector();
    std::unordered_map<player_id_t, Player> read_players_map();
    std::unordered_map<player_id_t, score_t> read_player_scores_map();

private:
    //throws length_error when wrong length
    void check_size(buffer_size_t needed_size);
};

// Class for storing and parsing outgoing message,
// supposed to keep at most only one message at the time
class OutputBuffer : public Buffer {
public:
    explicit OutputBuffer(DrawMessage::draw_message_variant &msg);
    explicit OutputBuffer(ClientMessage::client_message_variant &msg);
    explicit OutputBuffer(ServerMessage::server_message_variant &msg);

    buffer_size_t size();
    uint8_t *get_buffer();

protected:
    buffer_size_t write_index;

    void write_uint8_t(uint8_t number);
    void write_uint16_t(uint16_t number);
    void write_uint32_t(uint32_t number);
    void write_string(std::string &string);

    void write_player(Player &player);
    void write_position(Position &position);
    void write_bomb(Bomb &bomb);

    void write_bomb_placed_event(Event::BombPlacedEvent &event);
    void write_bomb_exploded_event(Event::BombExplodedEvent &event);
    void write_player_moved_event(Event::PlayerMovedEvent &event);
    void write_block_placed_event(Event::BlockPlacedEvent &event);

    void write_event(Event::event_message_variant &event);

    void write_players_id_vector(std::vector<player_id_t> &player_ids);
    void write_positions_vector(std::vector<Position> &positions);
    void write_bombs_vector(std::vector<Bomb> &bombs);
    void write_events_vector(std::vector<Event::event_message_variant> &events);

    void write_players_map(std::unordered_map<player_id_t, Player> &players);
    void write_player_positions_map(std::unordered_map<player_id_t, Position> &player_positions);
    void write_player_scores_map(std::unordered_map<player_id_t, score_t> &scores);

    void write_draw_lobby_message(DrawMessage::Lobby &msg);
    void write_draw_game_message(DrawMessage::Game &msg);

    void write_client_join_message(ClientMessage::Join &msg);
    void write_client_place_bomb_message();
    void write_client_place_block_message();
    void write_client_move_message(ClientMessage::Move &msg);

    void write_server_hello_message(ServerMessage::Hello &msg);
    void write_server_accepted_player_message(ServerMessage::AcceptedPlayer &msg);
    void write_server_game_started_message(ServerMessage::GameStarted &msg);
    void write_server_turn_message(ServerMessage::Turn &msg);
    void write_server_game_ended_message(ServerMessage::GameEnded &msg);
};

// Class for storing and parsing incoming messages by UDP protocol,
// supposed to keep at most only one message at the time
class UdpInputBuffer : public InputBuffer {
public:
    UdpInputBuffer();

    InputMessage::input_message_variant read_input_message();
    void add_packet(std::vector<uint8_t> &data, buffer_size_t size) override;

private:
    InputMessage::Move read_input_move_message();
    static InputMessage::PlaceBomb read_input_place_bomb_message();
    static InputMessage::PlaceBlock read_input_place_block_message();

    void reset_buffer();
};

// Class for storing and parsing incoming messages by TCP protocol
class TcpInputBuffer : public InputBuffer {
public:
    TcpInputBuffer();

    ServerMessage::server_message_variant read_server_message();
    ClientMessage::client_message_variant read_client_message();
    void add_packet(std::vector<uint8_t> &data, buffer_size_t size) override;

private:
    ServerMessage::Hello read_server_hello_message();
    ServerMessage::AcceptedPlayer read_server_accepted_player_message();
    ServerMessage::GameStarted read_server_game_started_message();
    ServerMessage::Turn read_server_turn_message();
    ServerMessage::GameEnded read_server_game_ended_message();

    ClientMessage::Join read_client_join_message();
    ClientMessage::PlaceBomb read_client_place_bomb_message();
    ClientMessage::PlaceBlock read_client_place_block_message();
    ClientMessage::Move read_client_move_message();

    void clean_after_correct_read();
};

#endif //ROBOTS_BUFFER_H
