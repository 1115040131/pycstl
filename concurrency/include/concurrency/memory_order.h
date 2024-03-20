#pragma once

#include <atomic>

namespace pyc {
namespace concurrency {

struct StdMemoryOrder {
    static constexpr auto memory_order_relaxed = std::memory_order_relaxed;
    static constexpr auto memory_order_consume = std::memory_order_consume;
    static constexpr auto memory_order_acquire = std::memory_order_acquire;
    static constexpr auto memory_order_release = std::memory_order_release;
    static constexpr auto memory_order_acq_rel = std::memory_order_acq_rel;
    static constexpr auto memory_order_seq_cst = std::memory_order_seq_cst;
};

struct DefaultMemoryOrder {
    static constexpr auto memory_order_relaxed = std::memory_order_seq_cst;
    static constexpr auto memory_order_consume = std::memory_order_seq_cst;
    static constexpr auto memory_order_acquire = std::memory_order_seq_cst;
    static constexpr auto memory_order_release = std::memory_order_seq_cst;
    static constexpr auto memory_order_acq_rel = std::memory_order_seq_cst;
    static constexpr auto memory_order_seq_cst = std::memory_order_seq_cst;
};

}  // namespace concurrency
}  // namespace pyc
