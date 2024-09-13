#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace pyc {

template <typename Connection>
class ConnectionPool {
public:
    ~ConnectionPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while (!connections_.empty()) {
            connections_.pop();
        }
    }

    void Close() {
        stop_ = true;
        cond_.notify_all();
    }

    std::optional<Connection> GetConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !connections_.empty() || stop_; });

        if (stop_) {
            return std::nullopt;
        }

        auto connection = std::move(connections_.front());
        connections_.pop();
        return connection;
    }

    void ReturnConnection(Connection&& connection) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (stop_) {
            return;
        }

        connections_.push(std::move(connection));
        cond_.notify_one();
    }

protected:
    std::atomic<bool> stop_{false};
    std::queue<Connection> connections_;
    std::condition_variable cond_;
    std::mutex mutex_;
};

}  // namespace pyc
