#ifndef ROBOTS_TCP_INCOMING_BUFFER_H
#define ROBOTS_TCP_INCOMING_BUFFER_H

#include "../structures.h"
#include "incoming_buffer.h"

// Class for storing and parsing incoming messages by TCP protocol
class TcpIncomingBuffer : public IncomingBuffer {
public:
    TcpIncomingBuffer();

    ServerMessage::server_message read_server_message();
    ClientMessage::client_message read_client_message();
    void add_packet(std::vector<uint8_t> &data, buffer_size_t size) override;

private:
    ServerMessage::Hello read_server_hello_message();
    ServerMessage::AcceptedPlayer read_server_accepted_player_message();
    ServerMessage::GameStarted read_server_game_started_message();
    ServerMessage::Turn read_server_turn_message();
    ServerMessage::GameEnded read_server_game_ended_message();

    ClientMessage::Join read_client_join_message();
    static ClientMessage::PlaceBomb read_client_place_bomb_message();
    static ClientMessage::PlaceBlock read_client_place_block_message();
    ClientMessage::Move read_client_move_message();

    void clean_after_correct_read();
};

#endif //ROBOTS_TCP_INCOMING_BUFFER_H
