#pragma once

#include <atomic>
#include <compare>
#include <cstdint>
#include <unordered_set>

#include "reaction/concept.h"

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

using NodeWeak = std::weak_ptr<ObserverNode>;

}  // namespace pyc::reaction

namespace std {

template <>
struct hash<pyc::reaction::UniqueID> {
    size_t operator()(const pyc::reaction::UniqueID& uid) const noexcept {
        return std::hash<uint64_t>()(static_cast<uint64_t>(uid));
    }
};

struct WeakPtrHash {
    size_t operator()(const pyc::reaction::NodeWeak& wp) const noexcept {
        return std::hash<pyc::reaction::ObserverNode*>()(wp.lock().get());
    }
};

struct WeakPtrEqual {
    bool operator()(const pyc::reaction::NodeWeak& lhs, const pyc::reaction::NodeWeak& rhs) const noexcept {
        return lhs.lock() == rhs.lock();
    }
};

}  // namespace std

namespace pyc::reaction {

using NodeSet = std::unordered_set<NodeWeak, std::WeakPtrHash, std::WeakPtrEqual>;
using NodeSetRef = std::reference_wrapper<NodeSet>;

}  // namespace pyc::reaction