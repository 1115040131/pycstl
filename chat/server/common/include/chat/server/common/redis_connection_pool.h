#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

#include <hiredis.h>

#include "logger/logger.h"

namespace pyc {
namespace chat {

/// @brief redisReply* 包装器, 自动调用 freeReplyObject 释放资源
struct RedisReplyGuard {
    explicit RedisReplyGuard(redisReply* reply) : reply_(reply) {}

    RedisReplyGuard(redisContext* context, std::string_view command)
        : reply_(static_cast<redisReply*>(redisCommand(context, command.data()))) {}

    ~RedisReplyGuard() { freeReplyObject(reply_); }

    redisReply* reply_;
};

class RedisConnectionPool {
    friend class Connection;

    /// @brief redisContext* 包装器, 自动调用 redisFree 释放资源
    using ConnectionImpl = std::unique_ptr<redisContext, void (*)(redisContext*)>;

public:
    RedisConnectionPool(size_t size, std::string_view host, int port, std::string_view password);

    ~RedisConnectionPool();

    void Close();

    /// @brief ConnectionImpl 包装器, 自动调用 ReturnConnection 回收连接
    class Connection {
    public:
        Connection(ConnectionImpl connection, RedisConnectionPool* pool) noexcept
            : connection_(std::move(connection)), pool_(pool) {}

        Connection(Connection&& other) noexcept : connection_(std::move(other.connection_)), pool_(other.pool_) {
            other.pool_ = nullptr;
        }

        Connection& operator=(Connection&& other) noexcept {
            if (this != &other) {
                Release();
                connection_ = std::move(other.connection_);
                pool_ = other.pool_;
                other.pool_ = nullptr;
            }
            return *this;
        }

        ~Connection() { Release(); }

        redisContext* Get() const { return connection_.get(); }

        void Release() {
            if (connection_ && pool_) {
                pool_->ReturnConnection(std::move(connection_));
            }
        }

    private:
        ConnectionImpl connection_;
        RedisConnectionPool* pool_;
    };

    std::optional<Connection> GetConnection();

private:
    void ReturnConnection(ConnectionImpl connection);

private:
    std::atomic<bool> stop_{false};
    std::queue<ConnectionImpl> connections_;
    std::condition_variable cond_;
    std::mutex mutex_;
};

inline Logger _g_redis_mgr_logger("RedisMgr");

inline bool RedisReplyOk(redisReply* reply) {
    return reply && reply->type == REDIS_REPLY_STATUS && ::strcasecmp(reply->str, "OK") == 0;
}

}  // namespace chat
}  // namespace pyc
