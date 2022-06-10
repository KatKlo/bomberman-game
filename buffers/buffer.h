#ifndef ROBOTS_BUFFER_H
#define ROBOTS_BUFFER_H

#include "../structures.h"
#include <vector>
#include <iostream>

// Superclass for all buffers-parsers
class Buffer {
public:
    using buffer_size_t = size_t;
    static constexpr auto MAX_PACKET_LENGTH = 65507;

    friend std::ostream &operator<<(std::ostream &os, const Buffer &buffer) {
        for (size_t i = 0; i < buffer.size_; i++) {
            os << (uint32_t) buffer.buffer_[i] << " ";
        }
        return os;
    }

protected:
    std::vector<uint8_t> buffer_;
    buffer_size_t capacity_;
    buffer_size_t size_;

    explicit Buffer(buffer_size_t capacity) : buffer_(capacity), capacity_(capacity), size_(0) {}
    void resize_if_needed(buffer_size_t needed_size) {
        if (needed_size > capacity_) {
            capacity_ = needed_size;
            buffer_.resize(capacity_, 0);
        }
    }
};

#endif //ROBOTS_BUFFER_H
