#ifndef ROBOTS_INCOMING_BUFFER_H
#define ROBOTS_INCOMING_BUFFER_H

#include "../structures.h"
#include <vector>
#include <unordered_map>
#include "buffer.h"

// Class for storing and parsing incoming messages
class IncomingBuffer : public Buffer {
public:
    virtual void add_packet(std::vector<uint8_t> &data, buffer_size_t size) = 0;

protected:
    buffer_size_t read_index;

    IncomingBuffer();
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

    Event::event_message read_event();

    std::vector<Event::event_message> read_events_vector();
    std::vector<player_id_t> read_players_id_vector();
    std::vector<Position> read_positions_vector();
    std::unordered_map<player_id_t, Player> read_players_map();
    std::unordered_map<player_id_t, score_t> read_player_scores_map();

private:
    //throws length_error when wrong length
    void check_size(buffer_size_t needed_size);
};


#endif //ROBOTS_INCOMING_BUFFER_H
