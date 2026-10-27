#include "core/conversation.h"
#include <limits>
#include <stdexcept>
#include <utility>

Conversation::Conversation() noexcept
    : messages_(nullptr), size_(0), capacity_(0) {}

Conversation::~Conversation() {
    delete[] messages_;
}

Conversation::Conversation(const Conversation& other)
    : Conversation() {
    if (other.size_ == 0) return;

    Message* copy = new Message[other.size_];
    try {
        for (std::size_t i = 0; i < other.size_; ++i) {
            copy[i] = other.messages_[i];
        }
    } catch (...) {
        delete[] copy;
        throw;
    }
    messages_ = copy;
    size_ = other.size_;
    capacity_ = other.size_;
}

Conversation& Conversation::operator=(const Conversation& other) {
    if (this != &other) {
        // Finish the copy before replacing anything we already own.
        Conversation copy(other);
        std::swap(messages_, copy.messages_);
        std::swap(size_, copy.size_);
        std::swap(capacity_, copy.capacity_);
    }
    return *this;
}

Conversation::Conversation(Conversation&& other) noexcept
    : messages_(other.messages_), size_(other.size_), capacity_(other.capacity_) {
    other.messages_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

Conversation& Conversation::operator=(Conversation&& other) noexcept {
    if (this != &other) {
        delete[] messages_;
        messages_ = other.messages_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.messages_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    return *this;
}

void Conversation::append(const Message& message) {
    if (size_ < capacity_) {
        messages_[size_] = message;
        ++size_;
        return;
    }

    const std::size_t max_capacity =
        std::numeric_limits<std::size_t>::max() / sizeof(Message);
    if (capacity_ > max_capacity / 2) {
        throw std::length_error("Conversation is too large");
    }
    std::size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
    Message* grown = new Message[new_capacity];
    try {
        for (std::size_t i = 0; i < size_; ++i) {
            grown[i] = messages_[i];
        }
        // message may refer to an element in our current array.
        grown[size_] = message;
    } catch (...) {
        delete[] grown;
        throw;
    }
    delete[] messages_;
    messages_ = grown;
    capacity_ = new_capacity;
    ++size_;
}

Message& Conversation::at(std::size_t index) {
    if (index >= size_) throw std::out_of_range("Conversation index out of range");
    return messages_[index];
}

const Message& Conversation::at(std::size_t index) const {
    if (index >= size_) throw std::out_of_range("Conversation index out of range");
    return messages_[index];
}
