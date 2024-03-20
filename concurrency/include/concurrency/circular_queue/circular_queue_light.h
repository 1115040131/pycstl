#pragma once

#include <atomic>
#include <memory>
#include <optional>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

/// @brief 通过原子变量实现可以在 Push 过程中 Pop 的环形并发队列
template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
class CircularQueueLight : public Noncopyable {
public:
    CircularQueueLight() : capacity_(N + 1), data_(alloc_.allocate(capacity_)) {}

    ~CircularQueueLight() {
        // 循环销毁
        while (head_ != tail_) {
            std::destroy_at(data_ + head_);
            head_ = (head_ + 1) % capacity_;
        }
        alloc_.deallocate(data_, capacity_);
    }

    bool Push(const T& value) {
        std::size_t tail;
        do {
            tail = tail_.load();
            // 队列已满
            if ((tail + 1) % capacity_ == head_.load()) {
                return false;
            }
        } while (!tail_.compare_exchange_strong(tail, (tail + 1) % capacity_));

        data_[tail] = value;
        std::size_t tailup;
        do {
            tailup = tail;
        } while (!tail_update_.compare_exchange_strong(tailup, (tailup + 1) % capacity_));
        return true;
    }

    std::optional<T> Pop() {
        std::size_t head;
        T result;
        do {
            head = head_.load();
            // 队列为空
            if (head == tail_.load()) {
                return {};
            }
            // 尾部未更新完
            if (head == tail_update_.load()) {
                return {};
            }
            result = data_[head];  // 这里拷贝防止多线程同时 pop 破坏队列数据
        } while (!head_.compare_exchange_strong(head, (head + 1) % capacity_));
        return result;
    }

private:
    std::size_t capacity_;
    T* data_;
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
    std::atomic<std::size_t> tail_update_{0};  // 正在更新的尾部数据
    [[no_unique_address]] Allocator alloc_;
};

}  // namespace concurrency
}  // namespace pyc