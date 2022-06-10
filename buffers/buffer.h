#ifndef ROBOTS_BUFFER_H
#define ROBOTS_BUFFER_H

#include "../structures.h"

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

#endif //ROBOTS_BUFFER_H
