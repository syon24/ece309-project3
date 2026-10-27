#pragma once

#include "core/message.h"
#include <cstddef>

class Conversation {
public:
    Conversation() noexcept;
    ~Conversation();
    Conversation(const Conversation& other);
    Conversation& operator=(const Conversation& other);
    Conversation(Conversation&& other) noexcept;
    Conversation& operator=(Conversation&& other) noexcept;

    void append(const Message& message);
    std::size_t size() const noexcept { return size_; }
    Message& at(std::size_t index);
    const Message& at(std::size_t index) const;

    Message* begin() noexcept { return messages_; }
    const Message* begin() const noexcept { return messages_; }
    Message* end() noexcept { return size_ == 0 ? messages_ : messages_ + size_; }
    const Message* end() const noexcept {
        return size_ == 0 ? messages_ : messages_ + size_;
    }

private:
    Message* messages_;
    std::size_t size_;
    std::size_t capacity_;
};
