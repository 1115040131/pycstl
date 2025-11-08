#pragma once

#include <atomic>
#include <compare>
#include <cstdint>

namespace pyc::reaction {

class UniqueID {
public:
    explicit UniqueID() : id_(generate()) {}

    operator uint64_t() const { return id_; }

    constexpr auto operator<=>(const UniqueID&) const = default;

private:
    uint64_t generate() {
        static std::atomic<uint64_t> counter{0};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    uint64_t id_;
};

}  // namespace pyc::reaction

namespace std {

template <>
struct hash<pyc::reaction::UniqueID> {
    size_t operator()(const pyc::reaction::UniqueID& uid) const noexcept {
        return std::hash<uint64_t>()(static_cast<uint64_t>(uid));
    }
};

}  // namespace std