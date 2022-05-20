#ifndef ROBOTS_BUFFER_H
#define ROBOTS_BUFFER_H

#include <string>
#include "structures.h"
#include <deque>
#include <ostream>

class Buffer {
public:
    using buffer_size_t = size_t;
    static constexpr auto MAX_PACKET_LENGTH = 65507;

    friend std::ostream &operator<<(std::ostream &os, const Buffer &buffer);

protected:
    void resize_if_needed(buffer_size_t needed_size);

    std::vector<uint8_t> buffer_;
    buffer_size_t capacity_;
    buffer_size_t size_;
};

class InputBuffer : public Buffer {
public:
    virtual void add_packet(std::vector<uint8_t> &data, buffer_size_t size) = 0;

protected:
    buffer_size_t read_index;

    void add_to_buffer(std::vector<uint8_t> &data, buffer_size_t size);

    uint8_t read_uint8_t();
    uint16_t read_uint16_t();
    uint32_t read_uint32_t();
    std::string read_string();

    Player read_player();
    Position read_position();

    ServerMessage::BombPlacedEvent read_bomb_placed_event();
    ServerMessage::BombExplodedEvent read_bomb_exploded_event();
    ServerMessage::PlayerMovedEvent read_player_moved_event();
    ServerMessage::BlockPlacedEvent read_block_placed_event();

    ServerMessage::event_message_variant read_event();

    std::vector<ServerMessage::event_message_variant> read_events_vector();
    std::vector<player_id_t> read_players_id_vector();
    std::vector<Position> read_positions_vector();
    std::unordered_map<player_id_t, Player> read_players_map();
    std::unordered_map<player_id_t, score_t> read_player_scores_map();

private:
    //throws length_error when wrong length
    void check_size(buffer_size_t needed_size);
};

// supposed to keep at most only one message at the time
class OutputBuffer : public Buffer {
public:
    buffer_size_t size();
    uint8_t *get_buffer();

    void write_client_to_GUI_message(ClientMessages::Client_GUI_message_variant &msg);
    void write_client_to_server_message(ClientMessages::Client_server_message_variant &msg);

protected:
    buffer_size_t write_index;

    void reset_buffer();

    void write_uint8_t(uint8_t number);
    void write_uint16_t(uint16_t number);
    void write_uint32_t(uint32_t number);
    void write_string(std::string &string);

    void write_player(Player &player);
    void write_position(Position &position);
    void write_bomb(Bomb &bomb);

    void write_positions_vector(std::vector<Position> &positions);
    void write_bombs_vector(std::vector<Bomb> &bombs);

    void write_players_map(std::unordered_map<player_id_t, Player> &players);
    void write_player_positions_map(std::unordered_map<player_id_t, Position> &player_positions);
    void write_player_scores_map(std::unordered_map<player_id_t, score_t> &scores);

    void write_client_lobby_message(ClientMessages::LobbyMessage &msg);
    void write_client_game_message(ClientMessages::GameMessage &msg);
    void write_client_join_message(ClientMessages::JoinMessage &msg);
    void write_client_place_bomb_message(ClientMessages::PlaceBombMessage &msg);
    void write_client_place_block_message(ClientMessages::PlaceBlockMessage &msg);
    void write_client_move_message(ClientMessages::MoveMessage &msg);
};

// supposed to keep at most only one message at the time
class UdpInputBuffer : public InputBuffer {
public:
    GUIMessages::GUI_message_variant read_GUI_message();
    void add_packet(std::vector<uint8_t> &data, buffer_size_t size) override;

private:
    GUIMessages::MoveMessage read_gui_move_message();
    GUIMessages::PlaceBombMessage read_gui_place_bomb_message();
    GUIMessages::PlaceBlockMessage read_gui_place_block_message();

    void reset_buffer();
};


class TcpInputBuffer : public InputBuffer {
public:
    ServerMessage::Server_message_variant read_server_message();
    void add_packet(std::vector<uint8_t> &data, buffer_size_t size);

private:
    ServerMessage::HelloMessage read_server_hello_message();
    ServerMessage::AcceptedPlayerMessage read_server_accepted_player_message();
    ServerMessage::GameStartedMessage read_server_game_started_message();
    ServerMessage::TurnMessage read_server_turn_message();
    ServerMessage::GameEndedMessage read_server_game_ended_message();

    void clean_after_correct_read();
};

#endif //ROBOTS_BUFFER_H
