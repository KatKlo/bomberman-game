#include "udp_incoming_buffer.h"

void UdpIncomingBuffer::add_packet(std::vector<uint8_t> &data, Buffer::buffer_size_t size) {
    reset_buffer();
    add_to_buffer(data, size);
}


void UdpIncomingBuffer::reset_buffer() {
    read_index = 0;
    size_ = 0;
}

InputMessage::input_message_variant UdpIncomingBuffer::read_input_message() {
    InputMessage::input_message_variant result;
    switch (read_uint8_t()) {
        case InputMessage::PLACE_BOMB: {
            result = read_input_place_bomb_message();
            break;
        }
        case InputMessage::PLACE_BLOCK: {
            result = read_input_place_block_message();
            break;
        }
        case InputMessage::MOVE: {
            result = read_input_move_message();
            break;
        }
        default: {
            throw std::invalid_argument("bad gui message type");
        }
    }

    if (read_index != size_) {
        throw std::invalid_argument("gui message too long");
    }

    reset_buffer();
    return result;
}

InputMessage::Move UdpIncomingBuffer::read_input_move_message() {
    auto direction_number = read_uint8_t();
    return InputMessage::Move{Direction{direction_number}};
}

InputMessage::PlaceBomb UdpIncomingBuffer::read_input_place_bomb_message() {
    return InputMessage::PlaceBomb{};
}

InputMessage::PlaceBlock UdpIncomingBuffer::read_input_place_block_message() {
    return InputMessage::PlaceBlock{};
}

UdpIncomingBuffer::UdpIncomingBuffer() : IncomingBuffer() {}

