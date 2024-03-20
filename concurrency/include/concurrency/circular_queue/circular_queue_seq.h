

#pragma once

#include <atomic>
#include <memory>
#include <optional>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
class CircularQueueSeq : public Noncopyable {
public:
    CircularQueueSeq() : capacity_(N + 1), data_(alloc_.allocate(capacity_)) {}

    ~CircularQueueSeq() {
        // 相当于自旋锁加锁
        bool use_expected = false;
        do {
            use_expected = false;
        } while (!atomic_using_.compare_exchange_strong(use_expected, true));

        // 循环销毁
        while (head_ != tail_) {
            std::destroy_at(data_ + head_);
            head_ = (head_ + 1) % capacity_;
        }
        alloc_.deallocate(data_, capacity_);

        // 相当于解锁
        do {
            use_expected = true;
        } while (!atomic_using_.compare_exchange_strong(use_expected, false));
    }

    template <typename... Args>
    bool Emplace(Args&&... args) {
        bool use_expected = false;
        do {
            use_expected = false;
        } while (!atomic_using_.compare_exchange_strong(use_expected, true));

        if ((tail_ + 1) % capacity_ == head_) {
            do {
                use_expected = true;
            } while (!atomic_using_.compare_exchange_strong(use_expected, false));
            return false;
        }
        std::construct_at(data_ + tail_, std::forward<Args>(args)...);
        tail_ = (tail_ + 1) % capacity_;

        do {
            use_expected = true;
        } while (!atomic_using_.compare_exchange_strong(use_expected, false));
        return true;
    }

    bool Push(const T& value) { return Emplace(value); }

    bool Push(T&& value) { return Emplace(std::move(value)); }

    std::optional<T> Pop() {
        bool use_expected = false;
        do {
            use_expected = false;
        } while (!atomic_using_.compare_exchange_strong(use_expected, true));

        if (head_ == tail_) {
            do {
                use_expected = true;
            } while (!atomic_using_.compare_exchange_strong(use_expected, false));
            return {};
        }
        T result = std::move(data_[head_]);
        head_ = (head_ + 1) % capacity_;

        do {
            use_expected = true;
        } while (!atomic_using_.compare_exchange_strong(use_expected, false));
        return result;
    }

private:
    std::size_t capacity_;
    T* data_;
    std::size_t head_{0};
    std::size_t tail_{0};
    std::atomic<bool> atomic_using_{false};
    [[no_unique_address]] Allocator alloc_;
};

}  // namespace concurrency
}  // namespace pyc