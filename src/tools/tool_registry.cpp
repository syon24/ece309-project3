#include "tools/tool_registry.h"

#include <cstdint>
#include <utility>

ToolRegistry::ToolRegistry() : buckets_(16) {}
ToolRegistry::~ToolRegistry() = default;

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return static_cast<std::size_t>(h);
}

void ToolRegistry::rehash() {
    std::vector<std::vector<Entry>> grown(buckets_.size() * 2);
    for (auto& bucket : buckets_) {
        for (auto& entry : bucket) {
            std::size_t index = hash(entry.name) % grown.size();
            grown[index].push_back(std::move(entry));
        }
    }
    buckets_.swap(grown);
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (tool == nullptr) return false;
    std::string name = tool->name();
    if (find(name) != nullptr) return false;

    // Grow before the next entry would make the load factor exceed 1.
    if (size_ == buckets_.size()) rehash();
    std::size_t index = hash(name) % buckets_.size();
    buckets_[index].push_back({std::move(name), std::move(tool)});
    ++size_;
    return true;
}

Tool* ToolRegistry::find(std::string_view name) const {
    std::size_t index = hash(name) % buckets_.size();
    for (const auto& entry : buckets_[index]) {
        if (entry.name == name) return entry.tool.get();
    }
    return nullptr;
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& bucket : buckets_) {
        for (const auto& entry : bucket) {
            fn(*entry.tool);
        }
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / buckets_.size();
}

std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;
    for (const auto& bucket : buckets_) {
        if (bucket.size() > longest) longest = bucket.size();
    }
    return longest;
}
