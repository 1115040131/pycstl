#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <string_view>

#include <hiredis.h>

#include "common/singleton.h"

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

class RedisMgr : public Singleton<RedisMgr> {
    friend class Singleton<RedisMgr>;

public:
    ~RedisMgr() { Close(); }

    void Close();

    std::optional<std::string> Get(std::string_view key);
    bool Set(std::string_view key, std::string_view value);

    bool LPush(std::string_view key, std::string_view value);
    std::optional<std::string> LPop(std::string_view key);
    bool RPush(std::string_view key, std::string_view value);
    std::optional<std::string> RPop(std::string_view key);

    bool HSet(std::string_view key, std::string_view field, std::string_view value);
    std::optional<std::string> HGet(std::string_view key, std::string_view field);

    bool Del(std::string_view key);

    bool Exists(std::string_view key);

private:
    RedisMgr();

private:
    std::unique_ptr<RedisConnectionPool> pool_;
};

}  // namespace chat
}  // namespace pyc
