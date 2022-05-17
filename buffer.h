#ifndef ROBOTS_BUFFER_H
#define ROBOTS_BUFFER_H

#include <string>
#include "structures.h"
#include <deque>

// stores and operates on buffer
class Buffer {
public:
    using buffer_size_t = size_t;
    static constexpr auto MAX_UDP_LENGTH = 65507;
    static constexpr auto MAX_STRING_LENGTH = 255;

    Buffer();

    buffer_size_t size();

    void set_size(buffer_size_t new_size);

    char *get_buffer();

    virtual void reset() = 0;

protected:
    uint8_t buffer[MAX_UDP_LENGTH];
    buffer_size_t size_;
};

// subclass for buffer for messages received from client
class InputBuffer : public Buffer {
public:
    void reset();

    GUIMessages::GUI_message_variant read_GUI_message();
    ServerMessage::Server_message_variant read_server_message();

private:
    buffer_size_t read_index;

    uint8_t read_uint8_t();
    uint16_t read_uint16_t();
    uint32_t read_uint32_t();
    uint64_t read_uint64_t();
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

    GUIMessages::MoveMessage read_gui_move_message();
    GUIMessages::PlaceBombMessage read_gui_place_bomb_message();
    GUIMessages::PlaceBlockMessage read_gui_place_block_message();

    ServerMessage::HelloMessage read_server_hello_message();
    ServerMessage::AcceptedPlayerMessage read_server_accepted_player_message();
    ServerMessage::GameStartedMessage read_server_game_started_message();
    ServerMessage::TurnMessage read_server_turn_message();
    ServerMessage::GameEndedMessage read_server_game_ended_message();
};

// subclass for buffer for messages to send
class OutputBuffer : public Buffer {
public:
    typedef std::deque<OutputBuffer> output_buffer_queue;

    void reset();

    void write_client_to_GUI_message(ClientMessages::Client_GUI_message_variant &msg);
    void write_client_to_server_message(ClientMessages::Client_server_message_variant &msg);

private:
    buffer_size_t write_index;

    void write_uint8_t(uint8_t number);
    void write_uint16_t(uint16_t number);
    void write_uint32_t(uint32_t number);
    void write_uint64_t(uint64_t number);
    void write_string(std::string &string);

    void write_player(Player &player);
    void write_position(Position &position);
    void write_bomb(Bomb &bomb);

    void write_positions_vector(std::vector<Position> &positions);
    void write_bombs_vector(std::vector<Bomb> &bombs);

    void write_players_map(std::unordered_map<player_id_t, Player> &players);
    void write_player_positions_map(std::unordered_map<player_id_t, Position> &player_positions);
    void write_player_scores_map(std::unordered_map<player_id_t, score_t> &scores);

    void write_client_join_message(ClientMessages::JoinMessage &msg);
    void write_client_place_bomb_message(ClientMessages::PlaceBombMessage &msg);
    void write_client_place_block_message(ClientMessages::PlaceBlockMessage &msg);
    void write_client_move_message(ClientMessages::MoveMessage &msg);

    void write_client_lobby_message(ClientMessages::LobbyMessage &msg);
    void write_client_game_message(ClientMessages::GameMessage &msg);
};


#endif //ROBOTS_BUFFER_H
