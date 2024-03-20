#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

/// @brief 通过加速实现环形并发队列
template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
class CircularQueueLock : public Noncopyable {
public:
    CircularQueueLock() : capacity_(N + 1), data_(alloc_.allocate(capacity_)) {}

    ~CircularQueueLock() {
        // 循环销毁
        std::lock_guard<std::mutex> lock(mtx_);
        while (head_ != tail_) {
            std::destroy_at(data_ + head_);
            head_ = (head_ + 1) % capacity_;
        }
        alloc_.deallocate(data_, capacity_);
    }

    template <typename... Args>
    bool Emplace(Args&&... args) {
        std::lock_guard<std::mutex> lock(mtx_);
        if ((tail_ + 1) % capacity_ == head_) {
            return false;
        }
        std::construct_at(data_ + tail_, std::forward<Args>(args)...);
        tail_ = (tail_ + 1) % capacity_;
        return true;
    }

    bool Push(const T& value) { return Emplace(value); }

    bool Push(T&& value) { return Emplace(std::move(value)); }

    std::optional<T> Pop() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (head_ == tail_) {
            return {};
        }
        T result = std::move(data_[head_]);
        head_ = (head_ + 1) % capacity_;
        return result;
    }

private:
    std::size_t capacity_;
    T* data_;
    std::size_t head_{0};
    std::size_t tail_{0};
    std::mutex mtx_;
    [[no_unique_address]] Allocator alloc_;
};

}  // namespace concurrency
}  // namespace pyc