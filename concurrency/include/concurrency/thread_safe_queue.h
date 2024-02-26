#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

namespace pyc {
namespace concurrency {

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue& other) {
        std::lock_guard<std::mutex> lock(other.mtx_);
        data_ = other.data_;
    }

    void Push(T new_value) {
        std::lock_guard<std::mutex> lock(mtx_);
        data_.push(std::move(new_value));
        data_cond_.notify_one();
    }

    T WaitAndPop() {
        std::unique_lock<std::mutex> lock(mtx_);
        data_cond_.wait(lock, [this] { return !data_.empty(); });
        T value = std::move(data_.front());
        data_.pop();
        return value;
    }

    std::optional<T> TryPop() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (data_.empty()) {
            return {};
        }
        T value = std::move(data_.front());
        data_.pop();
        return value;
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return data_.empty();
    }

private:
    mutable std::mutex mtx_;
    std::queue<T> data_;
    std::condition_variable data_cond_;
};

}  // namespace concurrency
}  // namespace pyc
