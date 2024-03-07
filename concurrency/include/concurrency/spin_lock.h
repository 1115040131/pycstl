#pragma once

#include <atomic>

namespace pyc {
namespace concurrency {

class SpinLock {
public:
    void Lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
        }
    }

    void Unlock() { flag_.clear(std::memory_order_release); }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

}  // namespace concurrency
}  // namespace pyc
