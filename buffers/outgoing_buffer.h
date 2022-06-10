#ifndef ROBOTS_OUTGOING_BUFFER_H
#define ROBOTS_OUTGOING_BUFFER_H

#include "../structures.h"
#include "buffer.h"
#include <vector>
#include <unordered_map>

// Class for storing and parsing outgoing message,
// supposed to keep at most only one message at the time
class OutgoingBuffer : public Buffer {
public:
    explicit OutgoingBuffer(DrawMessage::draw_message &msg);
    explicit OutgoingBuffer(ClientMessage::client_message &msg);
    explicit OutgoingBuffer(ServerMessage::server_message &msg);

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

    void write_event(Event::event_message &event);

    void write_players_id_vector(std::vector<player_id_t> &player_ids);
    void write_positions_vector(std::vector<Position> &positions);
    void write_bombs_vector(std::vector<Bomb> &bombs);
    void write_events_vector(std::vector<Event::event_message> &events);

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


#endif //ROBOTS_OUTGOING_BUFFER_H
