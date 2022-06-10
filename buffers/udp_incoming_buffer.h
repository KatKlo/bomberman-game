#ifndef ROBOTS_UDP_INCOMING_BUFFER_H
#define ROBOTS_UDP_INCOMING_BUFFER_H

#include "../structures.h"
#include "incoming_buffer.h"

// Class for storing and parsing incoming messages by UDP protocol,
// supposed to keep at most only one message at the time
class UdpIncomingBuffer : public IncomingBuffer {
public:
    UdpIncomingBuffer();

    InputMessage::input_message read_input_message();
    void add_packet(std::vector<uint8_t> &data, buffer_size_t size) override;

private:
    InputMessage::Move read_input_move_message();
    static InputMessage::PlaceBomb read_input_place_bomb_message();
    static InputMessage::PlaceBlock read_input_place_block_message();

    void reset_buffer();
};


#endif //ROBOTS_UDP_INCOMING_BUFFER_H
