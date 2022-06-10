#include "buffer.h"
#include <iostream>

std::ostream &operator<<(std::ostream &os, const Buffer &buffer) {
    for (size_t i = 0; i < buffer.size_; i++) {
        os << (uint32_t) buffer.buffer_[i] << " ";
    }
    return os;
}

void Buffer::resize_if_needed(Buffer::buffer_size_t needed_size) {
    if (needed_size > capacity_) {
        capacity_ = needed_size;
        buffer_.resize(capacity_, 0);
    }
}

Buffer::Buffer(unsigned long capacity) : buffer_(capacity), capacity_(capacity), size_(0) {}