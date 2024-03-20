#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace pyc {
namespace concurrency {

/// @brief 生产者消费者模型
template <typename T>
class Channel {
public:
    Channel(std::size_t capacity = 0) : capacity_(capacity) {}

    template <typename... Args>
    bool Send(Args&&... args) {
        std::unique_lock<std::mutex> lock(mtx_);
        producer_.wait(lock, [this]() {
            // 对于无缓冲的 channel, 应该等待直到有消费者准备好
            return (capacity_ == 0 && queue_.empty()) || queue_.size() < capacity_ || closed_;
        });

        if (closed_) {
            return false;
        }

        queue_.emplace(std::forward<Args>(args)...);
        consumer_.notify_one();
        return true;
    }

    std::optional<T> Receive() {
        std::unique_lock<std::mutex> lock(mtx_);
        consumer_.wait(lock, [this]() { return !queue_.empty() || closed_; });

        if (closed_ && queue_.empty()) {
            return {};
        }

        auto result = queue_.front();
        queue_.pop();
        producer_.notify_one();
        return result;
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mtx_);
        closed_ = true;
        producer_.notify_all();
        consumer_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable producer_;
    std::condition_variable consumer_;
    std::size_t capacity_;
    bool closed_ = false;
};

}  // namespace concurrency
}  // namespace pyc
