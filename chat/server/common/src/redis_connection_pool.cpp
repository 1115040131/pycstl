#include "chat/server/common/redis_connection_pool.h"

namespace pyc {
namespace chat {

inline void redisContextDeleter(redisContext* r) { redisFree(r); }

RedisConnectionPool::RedisConnectionPool(size_t size, std::string_view host, int port, std::string_view password) {
    for (size_t i = 0; i < size; i++) {
        auto context = ConnectionImpl(redisConnect(host.data(), port), redisContextDeleter);

        // 连接失败
        if (!context || context->err) {
            _g_redis_mgr_logger.error("Connect error: {}", context->errstr);
            continue;
        }

        if (!password.empty()) {
            auto command = fmt::format("AUTH {}", password);
            auto guard = RedisReplyGuard(context.get(), command);

            // 认证失败
            if (!RedisReplyOk(guard.reply_)) {
                _g_redis_mgr_logger.error("[{}] failed", command);
                continue;
            }
            _g_redis_mgr_logger.info("[{}] success", command);
        }

        connections_.push(std::move(context));
    }
}

RedisConnectionPool::~RedisConnectionPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    Close();
    while (!connections_.empty()) {
        connections_.pop();
    }
}

void RedisConnectionPool::Close() {
    stop_ = true;
    cond_.notify_all();
}

std::optional<RedisConnectionPool::Connection> RedisConnectionPool::GetConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return !connections_.empty() || stop_; });

    if (stop_) {
        return std::nullopt;
    }

    Connection connection(std::move(connections_.front()), this);
    connections_.pop();
    return connection;
}

void RedisConnectionPool::ReturnConnection(ConnectionImpl connection) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (stop_) {
        return;
    }

    connections_.push(std::move(connection));
    cond_.notify_one();
}

}  // namespace chat
}  // namespace pyc
