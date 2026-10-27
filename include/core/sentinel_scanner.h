#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class SentinelScanner {
public:
    struct Result {
        std::string safe_text;
        bool sentinel_found = false;
    };

    // An empty sentinel is invalid. After detection, all later input is ignored.
    explicit SentinelScanner(std::string sentinel = "<|end_conversation|>");
    Result feed(std::string_view chunk);
    Result flush();
    std::size_t pending_size() const noexcept { return pending_.size(); }

private:
    std::string sentinel_;
    std::string pending_;
    bool found_ = false;
};
