#include "core/sentinel_scanner.h"
#include <stdexcept>
#include <utility>

SentinelScanner::SentinelScanner(std::string sentinel)
    : sentinel_(std::move(sentinel)) {
    if (sentinel_.empty()) {
        throw std::invalid_argument("Sentinel cannot be empty");
    }
}

SentinelScanner::Result SentinelScanner::feed(std::string_view chunk) {
    Result result;
    result.sentinel_found = found_;
    if (found_) return result;

    for (char ch : chunk) {
        // Check completion before adding the last character to pending_.
        if (pending_.size() == sentinel_.size() - 1 && ch == sentinel_.back()) {
            pending_.clear();
            found_ = true;
            result.sentinel_found = true;
            return result;
        }

        // A full-length candidate that failed cannot start the sentinel.
        if (pending_.size() == sentinel_.size() - 1) {
            if (pending_.empty()) {
                result.safe_text += ch; // A one-character sentinel did not match.
                continue;
            }
            result.safe_text += pending_.front();
            pending_.erase(0, 1);
        }
        pending_ += ch;
        while (!pending_.empty() &&
               sentinel_.compare(0, pending_.size(), pending_) != 0) {
            result.safe_text += pending_.front();
            pending_.erase(0, 1);
        }
    }
    return result;
}

SentinelScanner::Result SentinelScanner::flush() {
    Result result;
    result.sentinel_found = found_;
    result.safe_text = pending_;
    pending_.clear();
    return result;
}
